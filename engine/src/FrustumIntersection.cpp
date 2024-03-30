#include <kengine/FrustumIntersection.hpp>
#include <kengine/Math.hpp>

void FrustumIntersection::set(const glm::mat4& m, bool allowTestSpheres) {
    float invl;
    nxX = m[0][3] + m[0][0]; nxY = m[1][3] + m[1][0]; nxZ = m[2][3] + m[2][0]; nxW = m[3][3] + m[3][0];
    if (allowTestSpheres) {
        invl = math::invsqrt(nxX * nxX + nxY * nxY + nxZ * nxZ);
        nxX *= invl; nxY *= invl; nxZ *= invl; nxW *= invl;
    }
    planes[0] = { nxX, nxY, nxZ, nxW };
    pxX = m[0][3] - m[0][0]; pxY = m[1][3] - m[1][0]; pxZ = m[2][3] - m[2][0]; pxW = m[3][3] - m[3][0];
    if (allowTestSpheres) {
        invl = math::invsqrt(pxX * pxX + pxY * pxY + pxZ * pxZ);
        pxX *= invl; pxY *= invl; pxZ *= invl; pxW *= invl;
    }
    planes[1] = { pxX, pxY, pxZ, pxW };
    nyX = m[0][3] + m[0][1]; nyY = m[1][3] + m[1][1]; nyZ = m[2][3] + m[2][1]; nyW = m[3][3] + m[3][1];
    if (allowTestSpheres) {
        invl = math::invsqrt(nyX * nyX + nyY * nyY + nyZ * nyZ);
        nyX *= invl; nyY *= invl; nyZ *= invl; nyW *= invl;
    }
    planes[2] = { nyX, nyY, nyZ, nyW };
    pyX = m[0][3] - m[0][1]; pyY = m[1][3] - m[1][1]; pyZ = m[2][3] - m[2][1]; pyW = m[3][3] - m[3][1];
    if (allowTestSpheres) {
        invl = math::invsqrt(pyX * pyX + pyY * pyY + pyZ * pyZ);
        pyX *= invl; pyY *= invl; pyZ *= invl; pyW *= invl;
    }
    planes[3] = { pyX, pyY, pyZ, pyW };
    nzX = m[0][3] + m[0][2]; nzY = m[1][3] + m[1][2]; nzZ = m[2][3] + m[2][2]; nzW = m[3][3] + m[3][2];
    if (allowTestSpheres) {
        invl = math::invsqrt(nzX * nzX + nzY * nzY + nzZ * nzZ);
        nzX *= invl; nzY *= invl; nzZ *= invl; nzW *= invl;
    }
    planes[4] = { nzX, nzY, nzZ, nzW };
    pzX = m[0][3] - m[0][2]; pzY = m[1][3] - m[1][2]; pzZ = m[2][3] - m[2][2]; pzW = m[3][3] - m[3][2];
    if (allowTestSpheres) {
        invl = math::invsqrt(pzX * pzX + pzY * pzY + pzZ * pzZ);
        pzX *= invl; pzY *= invl; pzZ *= invl; pzW *= invl;
    }
    planes[5] = { pzX, pzY, pzZ, pzW };
}

bool FrustumIntersection::testAab(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
    /*
     * This is an implementation of the "2.4 Basic intersection test" of the mentioned site.
     * It does not distinguish between partially inside and fully inside, though, so the test with the 'p' vertex is omitted.
     */
    return nxX * (nxX < 0 ? minX : maxX) + nxY * (nxY < 0 ? minY : maxY) + nxZ * (nxZ < 0 ? minZ : maxZ) >= -nxW &&
        pxX * (pxX < 0 ? minX : maxX) + pxY * (pxY < 0 ? minY : maxY) + pxZ * (pxZ < 0 ? minZ : maxZ) >= -pxW &&
        nyX * (nyX < 0 ? minX : maxX) + nyY * (nyY < 0 ? minY : maxY) + nyZ * (nyZ < 0 ? minZ : maxZ) >= -nyW &&
        pyX * (pyX < 0 ? minX : maxX) + pyY * (pyY < 0 ? minY : maxY) + pyZ * (pyZ < 0 ? minZ : maxZ) >= -pyW &&
        nzX * (nzX < 0 ? minX : maxX) + nzY * (nzY < 0 ? minY : maxY) + nzZ * (nzZ < 0 ? minZ : maxZ) >= -nzW &&
        pzX * (pzX < 0 ? minX : maxX) + pzY * (pzY < 0 ? minY : maxY) + pzZ * (pzZ < 0 ? minZ : maxZ) >= -pzW;
}