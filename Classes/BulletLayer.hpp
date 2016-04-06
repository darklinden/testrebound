#ifndef __BULLETLAYER_H__
#define __BULLETLAYER_H__

#include "cocos2d.h"
#include "BulletNode.hpp"

class BulletLayer : public cocos2d::Layer
{
public:
    //BulletLayer();
    ~BulletLayer();
    virtual bool init();
    virtual void onEnter();
    virtual void onExit();
    CREATE_FUNC(BulletLayer);
    
    static cocos2d::Scene* createScene()
    {
        // 'scene' is an autorelease object
        auto scene = cocos2d::Scene::create();
        
        // 'layer' is an autorelease object
        auto layer = BulletLayer::create();
        scene->addChild(layer);
        
        // return the scene
        return scene;
    };
    
    void addBullet(const cocos2d::Point& toPos);
    void gcBullet();
	
    cocos2d::Vector<BulletNode *> _runningBulletPool; // 正在运行的子弹池

};

#endif // __BULLETLAYER_H__
