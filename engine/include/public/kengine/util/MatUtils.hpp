#pragma once
#include <glm/glm.hpp>
#include <bit>
#include <stdexcept>

/// <summary>
/// Lots of functions ported from joml
/// </summary>
namespace matutils {

    // ex: NX = Negative X, PX = Positive X
    enum FrustumPlane {
        PLANE_NX,
        PLANE_PX,
        PLANE_NY,
        PLANE_PY,
        PLANE_NZ,
        PLANE_PZ
    };

    // ex: NX = Negative X, PX = Positive X
    enum FrustumCorner {
        CORNER_NXNYNZ,
        CORNER_PXNYNZ,
        CORNER_PXPYNZ,
        CORNER_NXPYNZ,
        CORNER_PXNYPZ,
        CORNER_NXNYPZ,
        CORNER_NXPYPZ,
        CORNER_PXPYPZ
    };

    /// <summary>
    /// ported from joml
    /// </summary>
    inline static glm::vec4 frustumPlane(const glm::mat4& matrix, FrustumPlane plane) {
        glm::vec4 dest;
        switch (plane) {
        case PLANE_NX:
            dest = glm::vec4(matrix[0][3] + matrix[0][0], matrix[1][3] + matrix[1][0], matrix[2][3] + matrix[2][0], matrix[3][3] + matrix[3][0]);
            break;
        case PLANE_PX:
            dest = glm::vec4(matrix[0][3] - matrix[0][0], matrix[1][3] - matrix[1][0], matrix[2][3] - matrix[2][0], matrix[3][3] - matrix[3][0]);
            break;
        case PLANE_NY:
            dest = glm::vec4(matrix[0][3] + matrix[0][1], matrix[1][3] + matrix[1][1], matrix[2][3] + matrix[2][1], matrix[3][3] + matrix[3][1]);
            break;
        case PLANE_PY:
            dest = glm::vec4(matrix[0][3] - matrix[0][1], matrix[1][3] - matrix[1][1], matrix[2][3] - matrix[2][1], matrix[3][3] - matrix[3][1]);
            break;
        case PLANE_NZ:
            dest = glm::vec4(matrix[0][3] + matrix[0][2], matrix[1][3] + matrix[1][2], matrix[2][3] + matrix[2][2], matrix[3][3] + matrix[3][2]);
            break;
        case PLANE_PZ:
            dest = glm::vec4(matrix[0][3] - matrix[0][2], matrix[1][3] - matrix[1][2], matrix[2][3] - matrix[2][2], matrix[3][3] - matrix[3][2]);
            break;
        default:
            throw std::runtime_error("Invalid plane index");
        }

        // normalize using only XYZ
        auto invLength = 1.0f / std::sqrt(std::fma(dest.x, dest.x, std::fma(dest.y, dest.y, dest.z * dest.z)));
        return dest *= invLength;
    }

    inline static glm::vec3 transformProject(const glm::mat4& matrix, const glm::vec3& vec) {
        glm::vec4 transformedVec = matrix * glm::vec4(vec, 1.0f);

        // for perspective division
        if (transformedVec.w != 0) {
            auto invW = 1.0f / transformedVec.w;
            transformedVec.x *= invW;
            transformedVec.y *= invW;
            transformedVec.z *= invW;
        }

        return glm::vec3(transformedVec);
    }

    /// <summary>
    /// ported from joml
    /// </summary>
    inline static void transformAab(const glm::mat4& m, float minX, float minY, float minZ, float maxX, float maxY, float maxZ, glm::vec3& outMin, glm::vec3& outMax) {
        float xax = m[0][0] * minX, xay = m[0][1] * minX, xaz = m[0][2] * minX;
        float xbx = m[0][0] * maxX, xby = m[0][1] * maxX, xbz = m[0][2] * maxX;
        float yax = m[1][0] * minY, yay = m[1][1] * minY, yaz = m[1][2] * minY;
        float ybx = m[1][0] * maxY, yby = m[1][1] * maxY, ybz = m[1][2] * maxY;
        float zax = m[2][0] * minZ, zay = m[2][1] * minZ, zaz = m[2][2] * minZ;
        float zbx = m[2][0] * maxZ, zby = m[2][1] * maxZ, zbz = m[2][2] * maxZ;
        float xminx, xminy, xminz, yminx, yminy, yminz, zminx, zminy, zminz;
        float xmaxx, xmaxy, xmaxz, ymaxx, ymaxy, ymaxz, zmaxx, zmaxy, zmaxz;
        if (xax < xbx) {
            xminx = xax;
            xmaxx = xbx;
        }
        else {
            xminx = xbx;
            xmaxx = xax;
        }
        if (xay < xby) {
            xminy = xay;
            xmaxy = xby;
        }
        else {
            xminy = xby;
            xmaxy = xay;
        }
        if (xaz < xbz) {
            xminz = xaz;
            xmaxz = xbz;
        }
        else {
            xminz = xbz;
            xmaxz = xaz;
        }
        if (yax < ybx) {
            yminx = yax;
            ymaxx = ybx;
        }
        else {
            yminx = ybx;
            ymaxx = yax;
        }
        if (yay < yby) {
            yminy = yay;
            ymaxy = yby;
        }
        else {
            yminy = yby;
            ymaxy = yay;
        }
        if (yaz < ybz) {
            yminz = yaz;
            ymaxz = ybz;
        }
        else {
            yminz = ybz;
            ymaxz = yaz;
        }
        if (zax < zbx) {
            zminx = zax;
            zmaxx = zbx;
        }
        else {
            zminx = zbx;
            zmaxx = zax;
        }
        if (zay < zby) {
            zminy = zay;
            zmaxy = zby;
        }
        else {
            zminy = zby;
            zmaxy = zay;
        }
        if (zaz < zbz) {
            zminz = zaz;
            zmaxz = zbz;
        }
        else {
            zminz = zbz;
            zmaxz = zaz;
        }
        outMin.x = xminx + yminx + zminx + m[3][0];
        outMin.y = xminy + yminy + zminy + m[3][1];
        outMin.z = xminz + yminz + zminz + m[3][2];
        outMax.x = xmaxx + ymaxx + zmaxx + m[3][0];
        outMax.y = xmaxy + ymaxy + zmaxy + m[3][1];
        outMax.z = xmaxz + ymaxz + zmaxz + m[3][2];
    }

    inline static void transformAab(const glm::mat4& m, glm::vec3& min, glm::vec3& max, glm::vec3& outMin, glm::vec3& outMax) {
        transformAab(m, min.x, min.y, min.z, max.x, max.y, max.z, outMin, outMax);
    }

    /// <summary>
    /// ported from joml
    /// </summary>
    inline static void frustumAabb(const glm::mat4& m, glm::vec3& min, glm::vec3& max) {
        float minX = std::numeric_limits<float>::infinity();
        float minY = std::numeric_limits<float>::infinity();
        float minZ = std::numeric_limits<float>::infinity();
        float maxX = -std::numeric_limits<float>::infinity();
        float maxY = -std::numeric_limits<float>::infinity();
        float maxZ = -std::numeric_limits<float>::infinity();
        for (int t = 0; t < 8; t++) {
            float x = ((t & 1) << 1) - 1.0f;
            float y = (((t >> 1) & 1) << 1) - 1.0f;
            float z = (((t >> 2) & 1) << 1) - 1.0f;
            float invW = 1.0f / (m[0][3] * x + m[1][3] * y + m[2][3] * z + m[3][3]);
            float nx = (m[0][0] * x + m[1][0] * y + m[2][0] * z + m[3][0]) * invW;
            float ny = (m[0][1] * x + m[1][1] * y + m[2][1] * z + m[3][1]) * invW;
            float nz = (m[0][2] * x + m[1][2] * y + m[2][2] * z + m[3][2]) * invW;
            minX = minX < nx ? minX : nx;
            minY = minY < ny ? minY : ny;
            minZ = minZ < nz ? minZ : nz;
            maxX = maxX > nx ? maxX : nx;
            maxY = maxY > ny ? maxY : ny;
            maxZ = maxZ > nz ? maxZ : nz;
        }
        min.x = minX;
        min.y = minY;
        min.z = minZ;
        max.x = maxX;
        max.y = maxY;
        max.z = maxZ;
    }

    inline static void frustumCorner(const glm::mat4& m, FrustumCorner corner, glm::vec3& point) {
        float d1, d2, d3;
        float n1x, n1y, n1z, n2x, n2y, n2z, n3x, n3y, n3z;
        switch (corner) {
        case CORNER_NXNYNZ: // left, bottom, near
            n1x = m[0][3] + m[0][0]; n1y = m[1][3] + m[1][0]; n1z = m[2][3] + m[2][0]; d1 = m[3][3] + m[3][0]; // left
            n2x = m[0][3] + m[0][1]; n2y = m[1][3] + m[1][1]; n2z = m[2][3] + m[2][1]; d2 = m[3][3] + m[3][1]; // bottom
            n3x = m[0][3] + m[0][2]; n3y = m[1][3] + m[1][2]; n3z = m[2][3] + m[2][2]; d3 = m[3][3] + m[3][2]; // near
            break;
        case CORNER_PXNYNZ: // right, bottom, near
            n1x = m[0][3] - m[0][0]; n1y = m[1][3] - m[1][0]; n1z = m[2][3] - m[2][0]; d1 = m[3][3] - m[3][0]; // right
            n2x = m[0][3] + m[0][1]; n2y = m[1][3] + m[1][1]; n2z = m[2][3] + m[2][1]; d2 = m[3][3] + m[3][1]; // bottom
            n3x = m[0][3] + m[0][2]; n3y = m[1][3] + m[1][2]; n3z = m[2][3] + m[2][2]; d3 = m[3][3] + m[3][2]; // near
            break;
        case CORNER_PXPYNZ: // right, top, near
            n1x = m[0][3] - m[0][0]; n1y = m[1][3] - m[1][0]; n1z = m[2][3] - m[2][0]; d1 = m[3][3] - m[3][0]; // right
            n2x = m[0][3] - m[0][1]; n2y = m[1][3] - m[1][1]; n2z = m[2][3] - m[2][1]; d2 = m[3][3] - m[3][1]; // top
            n3x = m[0][3] + m[0][2]; n3y = m[1][3] + m[1][2]; n3z = m[2][3] + m[2][2]; d3 = m[3][3] + m[3][2]; // near
            break;
        case CORNER_NXPYNZ: // left, top, near
            n1x = m[0][3] + m[0][0]; n1y = m[1][3] + m[1][0]; n1z = m[2][3] + m[2][0]; d1 = m[3][3] + m[3][0]; // left
            n2x = m[0][3] - m[0][1]; n2y = m[1][3] - m[1][1]; n2z = m[2][3] - m[2][1]; d2 = m[3][3] - m[3][1]; // top
            n3x = m[0][3] + m[0][2]; n3y = m[1][3] + m[1][2]; n3z = m[2][3] + m[2][2]; d3 = m[3][3] + m[3][2]; // near
            break;
        case CORNER_PXNYPZ: // right, bottom, far
            n1x = m[0][3] - m[0][0]; n1y = m[1][3] - m[1][0]; n1z = m[2][3] - m[2][0]; d1 = m[3][3] - m[3][0]; // right
            n2x = m[0][3] + m[0][1]; n2y = m[1][3] + m[1][1]; n2z = m[2][3] + m[2][1]; d2 = m[3][3] + m[3][1]; // bottom
            n3x = m[0][3] - m[0][2]; n3y = m[1][3] - m[1][2]; n3z = m[2][3] - m[2][2]; d3 = m[3][3] - m[3][2]; // far
            break;
        case CORNER_NXNYPZ: // left, bottom, far
            n1x = m[0][3] + m[0][0]; n1y = m[1][3] + m[1][0]; n1z = m[2][3] + m[2][0]; d1 = m[3][3] + m[3][0]; // left
            n2x = m[0][3] + m[0][1]; n2y = m[1][3] + m[1][1]; n2z = m[2][3] + m[2][1]; d2 = m[3][3] + m[3][1]; // bottom
            n3x = m[0][3] - m[0][2]; n3y = m[1][3] - m[1][2]; n3z = m[2][3] - m[2][2]; d3 = m[3][3] - m[3][2]; // far
            break;
        case CORNER_NXPYPZ: // left, top, far
            n1x = m[0][3] + m[0][0]; n1y = m[1][3] + m[1][0]; n1z = m[2][3] + m[2][0]; d1 = m[3][3] + m[3][0]; // left
            n2x = m[0][3] - m[0][1]; n2y = m[1][3] - m[1][1]; n2z = m[2][3] - m[2][1]; d2 = m[3][3] - m[3][1]; // top
            n3x = m[0][3] - m[0][2]; n3y = m[1][3] - m[1][2]; n3z = m[2][3] - m[2][2]; d3 = m[3][3] - m[3][2]; // far
            break;
        case CORNER_PXPYPZ: // right, top, far
            n1x = m[0][3] - m[0][0]; n1y = m[1][3] - m[1][0]; n1z = m[2][3] - m[2][0]; d1 = m[3][3] - m[3][0]; // right
            n2x = m[0][3] - m[0][1]; n2y = m[1][3] - m[1][1]; n2z = m[2][3] - m[2][1]; d2 = m[3][3] - m[3][1]; // top
            n3x = m[0][3] - m[0][2]; n3y = m[1][3] - m[1][2]; n3z = m[2][3] - m[2][2]; d3 = m[3][3] - m[3][2]; // far
            break;
        default:
            throw std::runtime_error("Somehow hit a bad corner??!"); //$NON-NLS-1$
        }
        float c23x = n2y * n3z - n2z * n3y;
        float c23y = n2z * n3x - n2x * n3z;
        float c23z = n2x * n3y - n2y * n3x;
        float c31x = n3y * n1z - n3z * n1y;
        float c31y = n3z * n1x - n3x * n1z;
        float c31z = n3x * n1y - n3y * n1x;
        float c12x = n1y * n2z - n1z * n2y;
        float c12y = n1z * n2x - n1x * n2z;
        float c12z = n1x * n2y - n1y * n2x;
        float invDot = 1.0f / (n1x * c23x + n1y * c23y + n1z * c23z);
        point.x = (-c23x * d1 - c31x * d2 - c12x * d3) * invDot;
        point.y = (-c23y * d1 - c31y * d2 - c12y * d3) * invDot;
        point.z = (-c23z * d1 - c31z * d2 - c12z * d3) * invDot;
    }
}
