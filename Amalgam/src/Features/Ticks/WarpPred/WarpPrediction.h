#pragma once
#include "../../../SDK/SDK.h"
#include "../../Backtrack/Backtrack.h"

class CWarpPrediction
{
private:
    float m_flWarpThreshold = 4096.0f; // 64^2 units, can be adjusted
    
    float m_flMinVelocity = 50.0f;
    
    float m_flMaxPredictionTime = 0.5f;
    
    std::unordered_map<int, Vec3> m_mPredictedPositions;
    
    std::unordered_map<int, Vec3> m_mMovementVectors;
    
    std::unordered_map<int, Vec3> m_mLastPositions;
    
    std::unordered_map<int, bool> m_mIsWarping;
    
    Vec3 CalculateMovementVector(const std::vector<TickRecord*>& vRecords);
    
    bool DetectWarping(int iIndex, const std::vector<TickRecord*>& vRecords);
    
    float CalculateAlignmentFactor(const Vec3& vAimDirection, const Vec3& vMovementVector);

public:
    void Initialize();
    
    void Update();
    
    bool PredictWarpPosition(int iIndex, Vec3& vPredictedPos, float flPredictionTime = 0.2f);
    
    float GetAlignmentFactor(int iIndex, const Vec3& vAimDirection);
    
    bool IsWarping(int iIndex);
    
    void Draw();
};

ADD_FEATURE(CWarpPrediction, WarpPrediction);