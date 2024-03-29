#pragma once

/// <summary>
/// Ported from the Java library JOML
/// </summary>
class FrustumIntersection {
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
};