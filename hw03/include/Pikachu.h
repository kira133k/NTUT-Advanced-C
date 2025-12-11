#ifndef Pikachu_h_
#define Pikachu_h_

#include "Object.h"
#include "Sphere.h"
#include <vector>
#include <cmath>
#include <algorithm>

// 皮卡丘複合物件 (High-Res Version)
class Pikachu : public Object
{
    struct SphereData
    {
        Vector3 c;
        float r;
    };

    std::vector<Sphere *> parts;
    std::vector<SphereData> partsData; // 備份資料用於計算法向量
    Vector3 centerPos;
    BBox bbox;

public:
    Pikachu(const Vector3 &pos, float scale) : centerPos(pos)
    {
        // --- 1. 頭部與身體 (Head & Body) ---
        // 臉稍微寬一點，比較可愛
        addSphere(pos + Vector3(0, 5.5, 0) * scale, 5.8f * scale);
        // 下盤臉頰肉 (讓臉看起來胖嘟嘟)
        addSphere(pos + Vector3(-3.5, 4, 2) * scale, 2.5f * scale);
        addSphere(pos + Vector3(3.5, 4, 2) * scale, 2.5f * scale);

        // 身體：梨形身材 (上窄下寬)
        addSphere(pos + Vector3(0, -3, 0) * scale, 5.0f * scale);
        addSphere(pos + Vector3(0, -5, 0) * scale, 5.5f * scale);

        // --- 2. 耳朵 (Ears) - 使用 addTube 畫出漸細的長耳朵 ---
        // 左耳：從頭頂往左上長，半徑從 1.5 縮小到 0.5
        addTube(pos + Vector3(-3, 9, 0) * scale,
                pos + Vector3(-8, 18, 1) * scale,
                1.5f * scale, 0.5f * scale, 8);

        // 右耳
        addTube(pos + Vector3(3, 9, 0) * scale,
                pos + Vector3(8, 18, 1) * scale,
                1.5f * scale, 0.5f * scale, 8);

        // --- 3. 臉部特徵 (Face) ---
        // 紅臉頰 (電力袋) - 稍微突出
        addSphere(pos + Vector3(-4.5, 3, 4.5) * scale, 1.6f * scale);
        addSphere(pos + Vector3(4.5, 3, 4.5) * scale, 1.6f * scale);

        // 眼睛 (Eyes)
        addSphere(pos + Vector3(-2, 6, 5.2) * scale, 0.7f * scale);
        addSphere(pos + Vector3(2, 6, 5.2) * scale, 0.7f * scale);

        // 鼻子 (Nose)
        addSphere(pos + Vector3(0, 5, 5.8) * scale, 0.3f * scale);

        // --- 4. 四肢 (Limbs) ---
        // 手 (Arms) - 短短的管狀
        addTube(pos + Vector3(-3, -2, 3.5) * scale,
                pos + Vector3(-1.5, -3, 5.5) * scale,
                1.2f * scale, 1.0f * scale, 4);
        addTube(pos + Vector3(3, -2, 3.5) * scale,
                pos + Vector3(1.5, -3, 5.5) * scale,
                1.2f * scale, 1.0f * scale, 4);

        // 腳 (Feet) - 橢圓形感覺 (用兩顆球疊)
        addSphere(pos + Vector3(-3.5, -9.5, 1) * scale, 1.8f * scale);
        addSphere(pos + Vector3(-4.5, -10, 3) * scale, 1.5f * scale);

        addSphere(pos + Vector3(3.5, -9.5, 1) * scale, 1.8f * scale);
        addSphere(pos + Vector3(4.5, -10, 3) * scale, 1.5f * scale);

        // --- 5. 閃電尾巴 (Zigzag Tail) ---
        // 利用 addTube 畫出折線，這是最難的部分
        float tailThick = 1.0f * scale;

        // 第一段：屁股往外
        addTube(pos + Vector3(0, -7, -4) * scale,
                pos + Vector3(3, -5, -5) * scale,
                0.8f * scale, tailThick, 4);

        // 第二段：往上折
        addTube(pos + Vector3(3, -5, -5) * scale,
                pos + Vector3(1, -1, -5) * scale,
                tailThick, tailThick, 4);

        // 第三段：再往外折
        addTube(pos + Vector3(1, -1, -5) * scale,
                pos + Vector3(4, 2, -5) * scale,
                tailThick, 1.5f * scale, 4);

        // 第四段：尾端大閃電 (變寬)
        addTube(pos + Vector3(4, 2, -5) * scale,
                pos + Vector3(2, 7, -6) * scale,
                1.5f * scale, 2.5f * scale, 6);
    }

    ~Pikachu()
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

        // 檢查每一個零件球
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
    // 基本加球函式
    void addSphere(Vector3 c, float r)
    {
        parts.push_back(new Sphere(c, r));
        partsData.push_back({c, r});

        if (parts.size() == 1)
            bbox = parts[0]->getBBox();
        else
            bbox.expandToInclude(parts.back()->getBBox());
    }

    // 進階：畫管狀物 (Tube) / 連續球體
    // p1: 起點, p2: 終點, r1: 起點半徑, r2: 終點半徑, steps: 切分多少顆球
    void addTube(Vector3 p1, Vector3 p2, float r1, float r2, int steps)
    {
        for (int i = 0; i <= steps; ++i)
        {
            float t = (float)i / steps;
            // 線性插值位置
            Vector3 pos = p1 * (1.0f - t) + p2 * t;
            // 線性插值半徑
            float r = r1 * (1.0f - t) + r2 * t;

            addSphere(pos, r);
        }
    }

    // 法向量計算
    Vector3 getNormalInternal(Vector3 hitPoint) const
    {
        float minError = 1e9;
        int bestIdx = 0;

        for (size_t i = 0; i < partsData.size(); ++i)
        {
            // 計算距離：在你的 Vector3 中，* 是內積，所以 sqrt(diff * diff) 是長度
            Vector3 diff = hitPoint - partsData[i].c;
            float dist = std::sqrt(diff * diff);

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