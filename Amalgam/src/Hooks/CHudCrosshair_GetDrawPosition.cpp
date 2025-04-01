#include "../SDK/SDK.h"

MAKE_SIGNATURE(CHudCrosshair_GetDrawPosition, "client.dll", "48 8B C4 55 53 56 41 54 41 55", 0x0);

MAKE_HOOK(CHudCrosshair_GetDrawPosition, S::CHudCrosshair_GetDrawPosition(), void,
    float* pX, float* pY, bool* pbBehindCamera, Vec3 angleCrosshairOffset)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CHudCrosshair_GetDrawPosition[DEFAULT_BIND])
		return CALL_ORIGINAL(pX, pY, pbBehindCamera, angleCrosshairOffset);
#endif

    auto pLocal = H::Entities.GetLocal();
    if (!pLocal) {
        return CALL_ORIGINAL(pX, pY, pbBehindCamera, angleCrosshairOffset);
    }

    bool bSet = false;

    // Handle Viewmodel Crosshair
    if (Vars::Visuals::Viewmodel::CrosshairAim.Value && pLocal->IsAlive())
    {
        Vec3 vScreen;

        // Check if there's a valid aim position
        if (!G::AimPosition.first.IsZero() && SDK::W2S(G::AimPosition.first, vScreen))
        {
            if (pX) *pX = vScreen.x;
            if (pY) *pY = vScreen.y;
            if (pbBehindCamera) *pbBehindCamera = false;
            bSet = true;
        }
    }

    // Handle Third-Person Crosshair
    if (Vars::Visuals::ThirdPerson::Crosshair.Value && !bSet && I::Input->CAM_IsThirdPerson())
    {
        const Vec3 viewangles = I::EngineClient->GetViewAngles();
        Vec3 vForward;
        Math::AngleVectors(viewangles, &vForward);


        const Vec3 vStartPos = pLocal->GetEyePosition();
        const Vec3 vEndPos = vStartPos + vForward * 8192;

        CGameTrace trace = {};
        CTraceFilterHitscan filter = {};
        filter.pSkip = pLocal;
        SDK::Trace(vStartPos, vEndPos, MASK_SHOT, &filter, &trace);

        Vec3 vScreen;
        if (SDK::W2S(trace.endpos, vScreen))
        {
            if (pX) *pX = vScreen.x;
            if (pY) *pY = vScreen.y;
            if (pbBehindCamera) *pbBehindCamera = false;
            bSet = true;
        }
    }

    // Fallback to the original implementation if no custom behavior is set
    if (!bSet)
    {
        CALL_ORIGINAL(pX, pY, pbBehindCamera, angleCrosshairOffset);
    }
}