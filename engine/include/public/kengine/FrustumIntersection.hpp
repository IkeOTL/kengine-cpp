#pragma once

/// <summary>
/// Ported from the Java library JOML
/// </summary>
class FrustumIntersection {
private:
    float nxX, nxY, nxZ, nxW;
    float pxX, pxY, pxZ, pxW;
    float nyX, nyY, nyZ, nyW;
    float pyX, pyY, pyZ, pyW;
    float nzX, nzY, nzZ, nzW;
    float pzX, pzY, pzZ, pzW;

public:
    // ex: NX = Negative X, PX = Positive X
    inline static const int CORNER_NXNYNZ = 0;
    inline static const int CORNER_PXNYNZ = 1;
    inline static const int CORNER_PXPYNZ = 2;
    inline static const int CORNER_NXPYNZ = 3;
    inline static const int CORNER_PXNYPZ = 4;
    inline static const int CORNER_NXNYPZ = 5;
    inline static const int CORNER_NXPYPZ = 6;
    inline static const int CORNER_PXPYPZ = 7;

    bool testAab(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
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
};