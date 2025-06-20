// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <primitives/block.h>
#include <uint256.h>
#include <logging.h>
#include <arith_uint256.h>
#include "validation.h"
#include "consensus/params.h"

#include <math.h>  // For pow()

unsigned int KimotoGravityWell(const CBlockIndex* pindexLast, const Consensus::Params& params) {
    const int64_t PastBlocksMin = 24;
    const int64_t PastBlocksMax = 24;

    if (pindexLast == nullptr || pindexLast->nHeight == 0) {
        LogPrintf("KGW: No previous block, returning powLimit\n");
        return UintToArith256(params.powLimit).GetCompact();
    }

    int64_t PastBlocksMass = 0;
    int64_t PastRateActualSeconds = 0;
    int64_t PastRateTargetSeconds = 0;
    double PastDifficultyAverage = 0;
    double PastDifficultyAveragePrev = 0;

    const CBlockIndex* BlockReading = pindexLast;

    for (int64_t i = 1; BlockReading && BlockReading->nHeight > 0 && i <= PastBlocksMax; i++) {
        if (i <= PastBlocksMin) {
            if (i == 1) {
                PastDifficultyAverage = double(BlockReading->nBits);
            } else {
                PastDifficultyAverage = ((double(BlockReading->nBits) - PastDifficultyAveragePrev) / i) + PastDifficultyAveragePrev;
            }
            PastDifficultyAveragePrev = PastDifficultyAverage;
        }

        PastBlocksMass++;

        int64_t BlockTime = BlockReading->GetBlockTime();
        int64_t PrevBlockTime = (BlockReading->pprev) ? BlockReading->pprev->GetBlockTime() : BlockTime;
        int64_t SolveTime = BlockTime - PrevBlockTime;

        if (SolveTime < 0)
            SolveTime = 0;
        if (SolveTime > 6 * params.nPowTargetSpacing)
            SolveTime = 6 * params.nPowTargetSpacing;

        PastRateActualSeconds += SolveTime;
        PastRateTargetSeconds += params.nPowTargetSpacing;

        BlockReading = BlockReading->pprev;
    }

    if (PastRateActualSeconds == 0 || PastRateTargetSeconds == 0) {
        LogPrintf("KGW: PastRateActualSeconds or PastRateTargetSeconds is zero, returning powLimit\n");
        return UintToArith256(params.powLimit).GetCompact();
    }

    double RateAdjustmentRatio = double(PastRateTargetSeconds) / double(PastRateActualSeconds);

    double EventHorizonDeviation = 1 + (0.7084 * pow((double(PastBlocksMass) / 28.2), -1.228));
    double EventHorizonDeviationFast = EventHorizonDeviation;
    double EventHorizonDeviationSlow = 1 / EventHorizonDeviation;

    if (RateAdjustmentRatio <= EventHorizonDeviationSlow || RateAdjustmentRatio >= EventHorizonDeviationFast) {
        LogPrintf("KGW: RateAdjustmentRatio %f outside event horizon, returning powLimit\n", RateAdjustmentRatio);
        return UintToArith256(params.powLimit).GetCompact();
    }

    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);

    bnNew = bnNew * RateAdjustmentRatio;

    if (bnNew == 0 || bnNew > UintToArith256(params.powLimit)) {
        bnNew = UintToArith256(params.powLimit);
    }

    LogPrintf("KGW: PastBlocksMass=%lld, PastRateActualSeconds=%lld, PastRateTargetSeconds=%lld\n",
              PastBlocksMass, PastRateActualSeconds, PastRateTargetSeconds);
    LogPrintf("KGW: RateAdjustmentRatio=%f, NewDifficulty=%s\n",
              RateAdjustmentRatio, bnNew.GetHex());

    return bnNew.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader* pblock, const Consensus::Params& params)
{
    return KimotoGravityWell(pindexLast, params);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    arith_uint256 bnNew;
    arith_uint256 bnOld;
    bnNew.SetCompact(pindexLast->nBits);
    bnOld = bnNew;
    // Litecoin: intermediate uint256 can overflow by 1 bit
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    bool fShift = bnNew.bits() > bnPowLimit.bits() - 1;
    if (fShift)
        bnNew >>= 1;
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;
    if (fShift)
        bnNew <<= 1;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
