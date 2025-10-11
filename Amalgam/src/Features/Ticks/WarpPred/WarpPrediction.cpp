#include "WarpPrediction.h"
#include "../../../Features/Backtrack/Backtrack.h"

// gods work by sean
void CWarpPrediction::Initialize()
{
    m_mPredictedPositions.clear();
    m_mMovementVectors.clear();
    m_mLastPositions.clear();
    m_mIsWarping.clear();
}

void CWarpPrediction::Update()
{
    auto pLocal = H::Entities.GetLocal();
    if (!pLocal || !pLocal->IsAlive())
        return;
    
    for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ENEMIES))
    {
        auto pPlayer = pEntity->As<CTFPlayer>();
        if (!pPlayer || pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost())
        {
            int iIndex = pPlayer->entindex();
            m_mPredictedPositions.erase(iIndex);
            m_mMovementVectors.erase(iIndex);
            m_mLastPositions.erase(iIndex);
            m_mIsWarping.erase(iIndex);
            continue;
        }
        
        int iIndex = pPlayer->entindex();
        
        std::vector<TickRecord*> vRecords;
        if (!F::Backtrack.GetRecords(pPlayer, vRecords) || vRecords.size() < 2)
        {
            m_mIsWarping[iIndex] = false;
            continue;
        }
        
        m_mLastPositions[iIndex] = pPlayer->m_vecOrigin();
        
        m_mMovementVectors[iIndex] = CalculateMovementVector(vRecords);
        
        m_mIsWarping[iIndex] = DetectWarping(iIndex, vRecords);
        
        if (m_mIsWarping[iIndex])
        {
            Vec3 vPredicted;
            if (PredictWarpPosition(iIndex, vPredicted))
                m_mPredictedPositions[iIndex] = vPredicted;
        }
    }
}

Vec3 CWarpPrediction::CalculateMovementVector(const std::vector<TickRecord*>& vRecords)
{
    if (vRecords.size() < 2)
        return Vec3();
    
    const int iSamplesToUse = std::min(5, (int)vRecords.size() - 1);
    
    Vec3 vTotalDelta;
    float flTotalTime = 0.0f;
    
    for (int i = 0; i < iSamplesToUse; i++)
    {
        Vec3 vDelta = vRecords[i]->m_vOrigin - vRecords[i + 1]->m_vOrigin;
        float flTimeDelta = vRecords[i]->m_flSimTime - vRecords[i + 1]->m_flSimTime;
        
        if (flTimeDelta > 0.0f)
        {
            vTotalDelta += vDelta;
            flTotalTime += flTimeDelta;
        }
    }
    
    Vec3 vVelocity = flTotalTime > 0.0f ? vTotalDelta / flTotalTime : Vec3();
    
    float flSpeed = sqrt(vVelocity.Length2DSqr());
    if (flSpeed > m_flMinVelocity)
        return vVelocity.Normalized();
    
    return Vec3();
}

bool CWarpPrediction::DetectWarping(int iIndex, const std::vector<TickRecord*>& vRecords)
{
    if (vRecords.size() < 2)
        return false;
    
    for (size_t i = 0; i < vRecords.size() - 1; i++)
    {
        Vec3 vDelta = vRecords[i]->m_vOrigin - vRecords[i + 1]->m_vOrigin;
        float flTimeDelta = vRecords[i]->m_flSimTime - vRecords[i + 1]->m_flSimTime;
        
        float flVelocity = flTimeDelta > 0.0f ? sqrt(vDelta.Length2DSqr()) / flTimeDelta : 0.0f;
        
        if (flVelocity > 500.0f)
            return true;
        
        if (vDelta.Length2DSqr() > m_flWarpThreshold && flTimeDelta < 0.1f)
            return true;
        
        if (flTimeDelta > 0.1f) // More than ~6-7 ticks
            return true;
    }
    
    return false;
}

bool CWarpPrediction::PredictWarpPosition(int iIndex, Vec3& vPredictedPos, float flPredictionTime)
{
    if (!m_mMovementVectors.contains(iIndex) || !m_mLastPositions.contains(iIndex))
        return false;
    
    auto pEntity = I::ClientEntityList->GetClientEntity(iIndex);
    if (!pEntity || pEntity->GetClientClass()->m_ClassID != (int)ETFClassID::CTFPlayer)
        return false;
    
    auto pPlayer = pEntity->As<CTFPlayer>();
    if (!pPlayer || !pPlayer->IsAlive())
        return false;
    
    Vec3 vVelocity = pPlayer->m_vecVelocity();
    float flSpeed = sqrt(vVelocity.Length2DSqr());
    
    if (flSpeed < m_flMinVelocity)
    {
        vVelocity = m_mMovementVectors[iIndex] * 300.0f; 
        flSpeed = 300.0f;
    }
    
    flPredictionTime = std::min(flPredictionTime, m_flMaxPredictionTime);
    
    vPredictedPos = m_mLastPositions[iIndex] + vVelocity * flPredictionTime;
    
    CGameTrace trace;
    CTraceFilterWorldAndPropsOnly filter;
    SDK::TraceHull(m_mLastPositions[iIndex], vPredictedPos, pPlayer->m_vecMins(), pPlayer->m_vecMaxs(), pPlayer->SolidMask(), &filter, &trace);
    
    if (trace.fraction < 1.0f)
    {
        vPredictedPos = trace.endpos;
        return true;
    }
    
    return true;
}

float CWarpPrediction::CalculateAlignmentFactor(const Vec3& vAimDirection, const Vec3& vMovementVector)
{

    Vec3 aimDir = vAimDirection;
    Vec3 moveVec = vMovementVector;
    return aimDir.Normalized().Dot(moveVec.Normalized());
}

float CWarpPrediction::GetAlignmentFactor(int iIndex, const Vec3& vAimDirection)
{
    if (!m_mMovementVectors.contains(iIndex))
        return 0.0f;
    
    return CalculateAlignmentFactor(vAimDirection, m_mMovementVectors[iIndex]);
}

bool CWarpPrediction::IsWarping(int iIndex)
{
    return m_mIsWarping.contains(iIndex) && m_mIsWarping[iIndex];
}

void CWarpPrediction::Draw()
{ 
    // Draw predicted positions for debugging
    for (const auto& [index, position] : m_mPredictedPositions)
    {
        if (m_mIsWarping[index])
        {
            // draw here for visualizatiaon
        }
    }
}