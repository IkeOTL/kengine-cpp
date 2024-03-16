#pragma once
#include <glm/glm.hpp>
#include <bit>

namespace vecutils {
    static size_t hashCode(glm::vec4 v) {
        const size_t prime = 31;
        size_t result = 1;
        result = prime * result + static_cast<std::size_t>(std::bit_cast<int>(v.w));
        result = prime * result + static_cast<std::size_t>(std::bit_cast<int>(v.x));
        result = prime * result + static_cast<std::size_t>(std::bit_cast<int>(v.y));
        result = prime * result + static_cast<std::size_t>(std::bit_cast<int>(v.z));
        return result;
    }

    static bool isFinite(const glm::vec3& v) {
        return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
    }

    static void transformAab(const glm::mat4& m, float minX, float minY, float minZ, float maxX, float maxY, float maxZ, glm::vec3& outMin, glm::vec3& outMax) {
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

    static void transformAab(const glm::mat4& m, glm::vec3& min, glm::vec3& max, glm::vec3& outMin, glm::vec3& outMax) {
        transformAab(m, min.x, min.y, min.z, max.x, max.y, max.z, outMin, outMax);
    }
}