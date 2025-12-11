#ifndef Doraemon_h_
#define Doraemon_h_

#include "Object.h"
#include "Sphere.h"
#include <vector>
#include <cmath>
#include <algorithm>

class Doraemon : public Object
{
    struct SphereData
    {
        Vector3 c;
        float r;
    };

    std::vector<Sphere *> parts;
    std::vector<SphereData> partsData;
    Vector3 centerPos;
    BBox bbox;

public:
    Doraemon(const Vector3 &pos, float scale) : centerPos(pos)
    {
        // --- 1. 頭部 (Head) ---
        // 藍色大頭
        addSphere(pos + Vector3(0, 10, 0) * scale, 10.0f * scale);
        // 白色臉孔區域 (稍微往前凸，讓臉有層次)
        addSphere(pos + Vector3(0, 8, 2.5) * scale, 8.5f * scale);

        // --- 2. 五官細節 ---
        // 眼睛 (Eyes) - 兩顆並排的大長圓型 (用兩顆球疊加模擬橢圓)
        // 左眼
        addSphere(pos + Vector3(-2.4, 14, 8.5) * scale, 2.4f * scale);
        addSphere(pos + Vector3(-2.4, 15.5, 8.0) * scale, 2.2f * scale); // 上半部稍微小一點
        // 右眼
        addSphere(pos + Vector3(2.4, 14, 8.5) * scale, 2.4f * scale);
        addSphere(pos + Vector3(2.4, 15.5, 8.0) * scale, 2.2f * scale);

        // 鼻子 (Nose) - 紅色小球
        addSphere(pos + Vector3(0, 13, 10.5) * scale, 1.5f * scale);

        // 鬍鬚 (Whiskers) - 左右各三根，這是靈魂！
        // 為了效能，我們每根鬍鬚只用 3-4 顆微小的球連成線
        float wLen = 4.0f * scale; // 鬍鬚長度
        float wR = 0.2f * scale;   // 鬍鬚粗細

        // 右邊鬍鬚
        addTube(pos + Vector3(3, 11, 9) * scale, pos + Vector3(3 + wLen, 12, 9) * scale, wR, wR, 3);
        addTube(pos + Vector3(3, 10, 9) * scale, pos + Vector3(3 + wLen, 10, 9) * scale, wR, wR, 3);
        addTube(pos + Vector3(3, 9, 9) * scale, pos + Vector3(3 + wLen, 8, 9) * scale, wR, wR, 3);

        // 左邊鬍鬚
        addTube(pos + Vector3(-3, 11, 9) * scale, pos + Vector3(-3 - wLen, 12, 9) * scale, wR, wR, 3);
        addTube(pos + Vector3(-3, 10, 9) * scale, pos + Vector3(-3 - wLen, 10, 9) * scale, wR, wR, 3);
        addTube(pos + Vector3(-3, 9, 9) * scale, pos + Vector3(-3 - wLen, 8, 9) * scale, wR, wR, 3);

        // --- 3. 身體 (Body) ---
        // 身體主幹 (藍色)
        addTube(pos + Vector3(0, 0, 0) * scale, pos + Vector3(0, -8, 0) * scale, 7.5f * scale, 7.0f * scale, 5);
        // 肚子 (白色區域)
        addSphere(pos + Vector3(0, -4, 4.5) * scale, 6.0f * scale);

        // 百寶袋 (Pocket) - 用半圈細微的球模擬袋子口
        addTube(pos + Vector3(-4, -4, 9.5) * scale, pos + Vector3(4, -4, 9.5) * scale, 0.3f * scale, 0.3f * scale, 8);

        // --- 4. 配件 (Accessories) ---
        // 項圈 (Collar) - 紅色帶子，圍繞脖子一圈
        // 我們用 addTube 畫一個多邊形環
        int collarSegments = 12;
        float collarRadius = 7.6f * scale;
        float collarY = 0.5f * scale;
        for (int i = 0; i < collarSegments; ++i)
        {
            float theta1 = (float)i / collarSegments * 6.2831853f;
            float theta2 = (float)(i + 1) / collarSegments * 6.2831853f;
            Vector3 p1 = pos + Vector3(cos(theta1) * collarRadius, collarY, sin(theta1) * collarRadius);
            Vector3 p2 = pos + Vector3(cos(theta2) * collarRadius, collarY, sin(theta2) * collarRadius);
            addTube(p1, p2, 0.6f * scale, 0.6f * scale, 2);
        }

        // 鈴鐺 (Bell) - 金色大球 + 黑色小孔
        addSphere(pos + Vector3(0, -1.5, 8.0) * scale, 2.0f * scale);
        // 鈴鐺中間的縫 (橫線)
        addTube(pos + Vector3(-1.5, -2, 9.5) * scale, pos + Vector3(1.5, -2, 9.5) * scale, 0.2f * scale, 0.2f * scale, 4);

        // --- 5. 四肢 (Limbs) ---
        // 手臂 (Arms)
        addTube(pos + Vector3(-7, 1, 0) * scale, pos + Vector3(-10, -2, 2) * scale, 2.0f * scale, 1.8f * scale, 4);
        addTube(pos + Vector3(7, 1, 0) * scale, pos + Vector3(10, -2, 2) * scale, 2.0f * scale, 1.8f * scale, 4);

        // 圓圓的手 (Hands)
        addSphere(pos + Vector3(-11, -3, 2.5) * scale, 2.8f * scale);
        addSphere(pos + Vector3(11, -3, 2.5) * scale, 2.8f * scale);

        // 腳 (Feet) - 扁平橢圓
        // 用「前後兩顆球」加上中間連接，模擬長橢圓形的腳
        // 左腳
        Vector3 lFootBack = pos + Vector3(-4, -14, -1) * scale;
        Vector3 lFootFront = pos + Vector3(-4, -14, 3) * scale;
        addTube(lFootBack, lFootFront, 2.8f * scale, 2.8f * scale, 3);

        // 右腳
        Vector3 rFootBack = pos + Vector3(4, -14, -1) * scale;
        Vector3 rFootFront = pos + Vector3(4, -14, 3) * scale;
        addTube(rFootBack, rFootFront, 2.8f * scale, 2.8f * scale, 3);

        // 稍微修飾兩腿中間的空隙，讓它不要看起來像浮在空中
        addSphere(pos + Vector3(0, -13, 0) * scale, 2.0f * scale);
    }

    ~Doraemon()
    {
        for (Sphere *s : parts)
            delete s;
        parts.clear();
    }

    bool getIntersection(const Ray &ray, IntersectionInfo *I) const override
    {
        bool hitAny = false;
        float closestT = 1e9;
        IntersectionInfo tempI;

        // 優化：先檢查整體的 BBox，如果光線完全沒碰到盒子，就不用檢查幾百顆球了
        float tmin, tmax;
        if (!bbox.intersect(ray, &tmin, &tmax))
            return false;

        for (const auto &part : parts)
        {
            if (part->getIntersection(ray, &tempI))
            {
                if (tempI.t < closestT)
                {
                    closestT = tempI.t;
                    *I = tempI;
                    I->object = this;
                    hitAny = true;
                }
            }
        }
        return hitAny;
    }

    Vector3 getNormal(const IntersectionInfo &I) const override
    {
        return getNormalInternal(I.hit);
    }

    BBox getBBox() const override
    {
        return bbox;
    }

    Vector3 getCentroid() const override
    {
        return centerPos;
    }

private:
    void addSphere(Vector3 c, float r)
    {
        parts.push_back(new Sphere(c, r));
        partsData.push_back({c, r});

        if (parts.size() == 1)
            bbox = parts[0]->getBBox();
        else
            bbox.expandToInclude(parts.back()->getBBox());
    }

    // 畫線/管子工具
    void addTube(Vector3 p1, Vector3 p2, float r1, float r2, int steps)
    {
        for (int i = 0; i <= steps; ++i)
        {
            float t = (float)i / steps;
            Vector3 pos = p1 * (1.0f - t) + p2 * t;
            float r = r1 * (1.0f - t) + r2 * t;
            addSphere(pos, r);
        }
    }

    Vector3 getNormalInternal(Vector3 hitPoint) const
    {
        float minError = 1e9;
        int bestIdx = 0;

        for (size_t i = 0; i < partsData.size(); ++i)
        {
            Vector3 diff = hitPoint - partsData[i].c;
            // 手動計算距離
            float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
            float dist = std::sqrt(distSq);

            float error = std::abs(dist - partsData[i].r);
            if (error < minError)
            {
                minError = error;
                bestIdx = i;
            }
        }
        return normalize(hitPoint - partsData[bestIdx].c);
    }
};

#endif