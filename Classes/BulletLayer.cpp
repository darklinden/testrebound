#include "BulletLayer.hpp"

USING_NS_CC;

void BulletLayer::onEnter()
{
    Layer::onEnter();
    
    // Register Touch Event
    auto listener = EventListenerTouchOneByOne::create();
    
    listener->onTouchBegan = [=] (Touch* t, Event* e) -> bool {
        auto p = t->getLocation();
                
        this->addBullet(p);
        return false;
    };
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    _touchListener = listener;
}

void BulletLayer::onExit()
{
    Layer::onExit();
    _eventDispatcher->removeEventListener(_touchListener);
}

BulletLayer::~BulletLayer()
{
    
}

bool BulletLayer::init()
{
    bool isRet = false;
    do
    {
        CC_BREAK_IF(!Layer::init());
        
        auto lb = Label::createWithSystemFont("", "", 20);
        lb->setAnchorPoint(Point::ANCHOR_BOTTOM_LEFT);
        lb->setPosition(Point(10, 100));
        this->addChild(lb, 1, 1);
        
        isRet = true;
    }
    while (0);
    return isRet;
}

void BulletLayer::addBullet(const cocos2d::Point &toPos)
{
    auto size = Director::getInstance()->getWinSize();
    cocos2d::Point startPoint(Point(size.width / 2, 50));
    
    std::vector<std::string> frames;
    frames.push_back("res/ball.png");
    
    auto bullet = BulletNode::create(startPoint, toPos, frames);
    if (bullet) {
        this->addChild(bullet);
        _runningBulletPool.pushBack(bullet);
        bullet->setVisible(true);
        bullet->runMove();
        
        auto lb = dynamic_cast<Label*>(this->getChildByTag(1));
        lb->setString(StringUtils::format("bullet count %ld", _runningBulletPool.size()));
    }
}

void BulletLayer::gcBullet()
{
    if (!_runningBulletPool.size()) return;
    
    for (int i = 0; i < _runningBulletPool.size(); i++)
    {
        auto bul = _runningBulletPool.at(i);
        bul->stopMove();
        _runningBulletPool.eraseObject(bul);
        i--;
    }
}
