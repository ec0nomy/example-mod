#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/cocos/include/cocos2d.h>
#include <random>
#include <ctime>
#include <cmath>

using namespace geode::prelude;
USING_NS_CC;

static const int CIRCLE_LAYER_TAG = 98421;

static CCNode* makeCleanCircle(float scale, GLubyte opacity)
{
    CCDrawNode* dn = CCDrawNode::create();
    const int SEGS = 128;
    const float RX = 140.f;
    const float RY = 60.f;
    CCPoint verts[SEGS];
    for (int s = 0; s < SEGS; s++) {
        float a = (float)s / (float)SEGS * 2.f * (float)M_PI;
        verts[s] = CCPoint(cosf(a) * RX, sinf(a) * RY);
    }
    float alpha = (float)opacity / 255.f;
    dn->drawPolygon(verts, SEGS,
        ccc4f(0.f, 0.f, 0.f, alpha),
        0.f,
        ccc4f(0.f, 0.f, 0.f, 0.f));
    CCNode* wrapper = CCNode::create();
    wrapper->addChild(dn);
    wrapper->setScale(scale);
    return wrapper;
}

static CCNode* makeSketchyCircle(float scale, GLubyte opacity)
{
    CCNode* wrapper = CCNode::create();
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> angleDist(-40.f, 40.f);
    std::uniform_real_distribution<float> rxVar(0.80f, 1.20f);
    std::uniform_real_distribution<float> ryVar(0.70f, 1.30f);
    std::uniform_real_distribution<float> alphaVar(0.55f, 0.90f);
    std::uniform_real_distribution<float> offsetVar(-8.f, 8.f);
    const int STROKES = 32;
    const float BASE_RX = 130.f;
    const float BASE_RY = 95.f;
    const int SEGS = 64;
    float baseAlpha = (float)opacity / 255.f;
    for (int i = 0; i < STROKES; i++) {
        CCDrawNode* stroke = CCDrawNode::create();
        float rx = BASE_RX * rxVar(rng);
        float ry = BASE_RY * ryVar(rng);
        CCPoint verts[SEGS];
        for (int s = 0; s < SEGS; s++) {
            float a = (float)s / (float)SEGS * 2.f * (float)M_PI;
            verts[s] = CCPoint(
                cosf(a) * rx + offsetVar(rng),
                sinf(a) * ry + offsetVar(rng)
            );
        }
        float alpha = baseAlpha * alphaVar(rng);
        stroke->drawPolygon(verts, SEGS,
            ccc4f(0.f, 0.f, 0.f, alpha),
            0.f,
            ccc4f(0.f, 0.f, 0.f, 0.f));
        stroke->setRotation(angleDist(rng));
        wrapper->addChild(stroke);
    }
    wrapper->setScale(scale);
    return wrapper;
}

static CCNode* makeCircle(float scale, GLubyte opacity, std::mt19937& rng)
{
    bool useSketchy = std::uniform_int_distribution<int>(0, 1)(rng) == 0;
    if (useSketchy)
        return makeSketchyCircle(scale, opacity);
    else
        return makeCleanCircle(scale, opacity);
}

class $modify(PlayLayer) {

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        auto* mod = Mod::get();

        if (!mod->getSettingValue<bool>("enabled"))
            return true;

        int   minC = (int)mod->getSettingValue<int64_t>("min-circles");
        int   maxC = (int)mod->getSettingValue<int64_t>("max-circles");
        float scl  = (float)mod->getSettingValue<double>("circle-scale");
        int   opc  = (int)mod->getSettingValue<int64_t>("circle-opacity");

        if (minC < 1) minC = 1;
        if (maxC < 1) maxC = 1;
        if (minC > maxC) std::swap(minC, maxC);

        unsigned seed = (unsigned)std::time(nullptr)
                      ^ (level ? (unsigned)level->m_levelID.value() : 0u);
        std::mt19937 rng(seed);

        int count = std::uniform_int_distribution<int>(minC, maxC)(rng);

        CCLayer* objLayer = m_objectLayer;
        if (!objLayer) objLayer = this;

        float levelLengthPx = 30000.f;
        if (level) {
            float ll = (float)level->m_levelLength;
            if (ll > 100.f) levelLengthPx = ll * 30.f;
        }

        CCLayer* circleLayer = CCLayer::create();
        circleLayer->setTag(CIRCLE_LAYER_TAG);

        float sectionWidth = (levelLengthPx - 1000.f) / (float)count;

        std::uniform_int_distribution<int> chancePicker(0, 9);
        std::uniform_real_distribution<float> groundY(60.f, 130.f);
        std::uniform_real_distribution<float> anyY(130.f, 350.f);

        for (int i = 0; i < count; i++) {
            float sectionStart = 500.f + i * sectionWidth;
            float sectionEnd   = sectionStart + sectionWidth;
            std::uniform_real_distribution<float> xDist(sectionStart, sectionEnd);

            float px = xDist(rng);
            float py;

            if (chancePicker(rng) < 7)
                py = groundY(rng);
            else
                py = anyY(rng);

            CCNode* circle = makeCircle(scl, (GLubyte)opc, rng);
            circle->setPosition(CCPoint(px, py));
            circleLayer->addChild(circle, -10);
        }

        objLayer->addChild(circleLayer, -10);
        return true;
    }
};
