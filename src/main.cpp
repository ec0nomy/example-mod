#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/cocos/include/cocos2d.h>
#include <random>
#include <ctime>

using namespace geode::prelude;
USING_NS_CC;

static const int CIRCLE_LAYER_TAG = 98421;

static CCSprite* makeCircleSprite(float scale, GLubyte opacity, std::mt19937& rng)
{
    bool useSketchy = std::uniform_int_distribution<int>(0, 1)(rng) == 0;

    const char* filename = useSketchy
        ? "circle_sketchy.jpg"
        : "circle_clean.png";

    CCSprite* spr = CCSprite::create(filename);

    if (!spr) {
        log::warn("DarkCirclesBuff: failed to load {}, using fallback", filename);
        return nullptr;
    }

    spr->setScale(scale);
    spr->setOpacity(opacity);
    return spr;
}

static CCNode* makeFallbackCircle(float scale, GLubyte opacity)
{
    CCDrawNode* dn = CCDrawNode::create();
    const int SEGS = 64;
    CCPoint verts[SEGS];
    for (int s = 0; s < SEGS; s++) {
        float a = (float)s / (float)SEGS * 2.f * (float)M_PI;
        verts[s] = CCPoint(cosf(a) * 100.f, sinf(a) * 50.f);
    }
    dn->drawPolygon(verts, SEGS,
        ccc4f(0,0,0, (float)opacity/255.f), 0.f, ccc4f(0,0,0,0));
    CCNode* wrapper = CCNode::create();
    wrapper->addChild(dn);
    wrapper->setScale(scale);
    return wrapper;
}

class $modify(PlayLayer) {

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        auto* mod = Mod::get();

        if (!mod->getSettingValue<bool>("enabled"))
            return true;

        int   minC    = (int)mod->getSettingValue<int64_t>("min-circles");
        int   maxC    = (int)mod->getSettingValue<int64_t>("max-circles");
        float scl     = (float)mod->getSettingValue<double>("circle-scale");
        int   opc     = (int)mod->getSettingValue<int64_t>("circle-opacity");
        int   vspread = (int)mod->getSettingValue<int64_t>("spread-vertical");

        if (minC < 1)  minC = 1;
        if (maxC < 1)  maxC = 1;
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

        std::uniform_real_distribution<float> xDist(300.f, levelLengthPx - 300.f);
        std::uniform_real_distribution<float> yDist(
            -(float)vspread * 0.5f,
             (float)vspread * 0.5f
        );
        const float Y_CENTER = 160.f;

        for (int i = 0; i < count; i++) {
            float px = xDist(rng);
            float py = Y_CENTER + yDist(rng);

            CCSprite* spr = makeCircleSprite(scl, (GLubyte)opc, rng);

            if (spr) {
                spr->setPosition(CCPoint(px, py));
                circleLayer->addChild(spr, -10);
            } else {
                CCNode* fb = makeFallbackCircle(scl, (GLubyte)opc);
                fb->setPosition(CCPoint(px, py));
                circleLayer->addChild(fb, -10);
            }
        }

        objLayer->addChild(circleLayer, -10);
        return true;
    }
};
