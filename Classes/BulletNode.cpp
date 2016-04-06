#include "BulletNode.hpp"

BulletNode *BulletNode::create(const cocos2d::Point &startPos,
                               const cocos2d::Point &pointPos,
                               const std::vector<std::string> animationFrames)
{
    BulletNode *bullet = new BulletNode();
    
    if (bullet && bullet->init(startPos,
                               pointPos,
                               animationFrames))
    {
        bullet->autorelease();
        return bullet;
    }
    CC_SAFE_DELETE(bullet);
    return nullptr;
}

bool BulletNode::init(const cocos2d::Point &startPos,
                      const cocos2d::Point &pointPos,
                      const std::vector<std::string> animationFrames)
{
    bool isRet = false;
    do
    {
        CC_BREAK_IF(!Node::init());
        
        this->setStartPos(startPos);
        this->setStartVec(posGetVec(startPos, pointPos));
        this->setCrossPos(Point::ZERO);
        this->setCrossVec(Point::ZERO);
        this->setMoveSpeed(1000);
        _animationFrames = animationFrames;
        
        CC_BREAK_IF(!_animationFrames.size());
        
        if (_sp) {
            _sp->removeFromParent();
            _sp = nullptr;
        }
        
        auto spf = SpriteFrameCache::getInstance()->getSpriteFrameByName(_animationFrames[0]);
        if (spf) {
            _sp = Sprite::createWithSpriteFrame(spf);
        }
        else {
            _sp = Sprite::create(_animationFrames[0]);
        }
        
        CC_BREAK_IF(!_sp);
        
        int32_t w = round(MAX(_sp->getContentSize().width, _sp->getContentSize().height));
        
        _sp->setScale(20.0 / w);
        
        this->setContentSize(Size(20, 20));
        
        this->addChild(_sp);
        
        isRet = true;
        
    } while (0);
    return isRet;
}

void BulletNode::playAnimation()
{
    // 播放闪烁动画
    auto key = _animationFrames[0];
    auto amn = AnimationCache::getInstance()->getAnimation(key);
    if (!amn) {
        Vector<cocos2d::SpriteFrame *> arrayOfSpriteFrameNames;
        
        for (int32_t i = 0; i < _animationFrames.size(); i++) {
            auto spf = SpriteFrameCache::getInstance()->getSpriteFrameByName(_animationFrames[i]);
            if (spf) {
                arrayOfSpriteFrameNames.pushBack(spf);
            }
        }
        
        amn = Animation::createWithSpriteFrames(arrayOfSpriteFrameNames);
        AnimationCache::getInstance()->addAnimation(amn, key);
    }
    
    auto ame = Animate::create(amn);
    _sp->runAction(RepeatForever::create(ame));
}

void BulletNode::runMove()
{
    this->stopAllActions();
    
    // 计算移动
    if (_cross_pos == Point::ZERO && _cross_vector == Point::ZERO) {
        // 初始化
        // pass
    }
    else {
        // 中间碰撞
        this->setStartPos(this->getCrossPos());
        this->setStartVec(this->getCrossVec());
    }
    
    // 计算当前状态下下一周期的碰撞
    this->setPosition(this->getStartPos());
    this->faceVec(this->getStartVec());
    
    auto r = Rect(0, 0, this->getParent()->getContentSize().width, this->getParent()->getContentSize().height);

    Point cross;
    Vec2 f;
    reboundOfRayInRect(this->getStartPos(),
                                  this->getStartVec(),
                                  r,
                       &cross,
                       &f);
    
    this->setCrossPos(cross);
    this->setCrossVec(f);
    
    auto distance = _cross_pos.distance(_start_pos);
    auto duration = distance / _move_speed;
    auto move = MoveTo::create(duration, this->getCrossPos());
    auto call = CallFunc::create([=] {
        this->runMove();
    });
    
    this->runAction(Sequence::create(move, call, NULL));
}

// 停止子弹动画
void BulletNode::stopMove()
{
    this->stopAllActions();
    this->setVisible(false);
    this->setRotation(0);
}

void BulletNode::faceVec(const cocos2d::Vec2& vec)
{
    // 计算对于正上方的偏转角
    auto angle_atanValue = atan2f(-vec.y, vec.x);
    
    //弧度转换成角度
    auto angle = angle_atanValue * 180 / M_PI;
    
    _sp->setRotation(angle + 90);
}

// 用起点和终点获取向量
cocos2d::Vec2 BulletNode::posGetVec(const cocos2d::Point& start,
                                    const cocos2d::Point& end)
{
    auto v = end - start;
    if (v.x > 100 && v.y > 100) {
        v = v / 10.0;
    }
    return v;
}

// 获取边界内最长线段长度
float BulletNode::maxLenInRect(const cocos2d::Rect& rect)
{
    auto s = rect.size;
    return s.width + s.height; // sqrtf((s.width * s.width) + (s.height * s.height));
}

// 起始点 pos 在向量 vec 上 延伸 len 达到的点, 用于交叉
cocos2d::Point BulletNode::posAddVectorLen(const cocos2d::Point& pos,
                                           const cocos2d::Vec2& vec,
                                           const float len)
{
    auto ret = pos;
    ret += vec * len;
    return ret;
}

// 使用向量计算反弹
cocos2d::Vec2 BulletNode::boundedVec(const cocos2d::Vec2& S,
                                     const cocos2d::Vec2& B)
{
    // 在游戏循环中
    // 移动的物体简化为质点,位置是x=0.0f,y=0.0f
    // 质点速度向量的分量是Svx=4.0f,Svy=2.0f
    // 障碍向量是bx=14.0f-6.0f=8.0f,by=4.0f-12.0f=-8.0f
    // 则障碍向量的垂直向量是Nx=-8.0f,Ny=-8.0f
    
    // 任给一个非零向量(x,y)，则它相对坐标轴逆时针转90度的垂直向量为(-y,x),顺时针转90度垂直向量为(y,-x)。
    auto N = cocos2d::Vec2(B.y, -B.x);
    
    // 这里可以加入碰撞检测
    // 现在假设已经碰撞完毕，开始反弹计算！
    
    // 计算N的长度
    float lengthN = sqrt((N.x * N.x) + (N.y * N.y)) ;
    // 归一化N为n'
    float n0x = N.x / lengthN ; // n0x就是n'的x分量
    float n0y = N.y / lengthN ; // n0y就是n'的y分量
    // 计算n，就是S在N方向上的投影向量
    // 根据b'= (-b.a1').a1'，有n = (-S.n').n'
    float nx = -((S.x * n0x) + (S.y * n0y)) * n0x ; // n的x分量
    float ny = -((S.x * n0x) + (S.y * n0y)) * n0y ; // n的y分量
    // 计算T
    // T = S + n
    float Tx = S.x + nx ; // T的x分量
    float Ty = S.y + ny ; // T的y分量
    // 有了T，有了F = 2*T - S，好了，你现在拥有一切了
    // 计算F
    float Fx = 2*Tx - S.x ; // F的x分量
    float Fy = 2*Ty - S.y ; // F的y分量
    // 现在已经计算出了反弹后的速度向量了
    // 更新速度向量
    return Vec2(Fx, Fy);
}

bool BulletNode::isPointOnLine(const cocos2d::Point& pos,
                               const cocos2d::Point& start,
                               const cocos2d::Point& end)
{
    auto va = start - end;
    auto vb = pos - end;
    //    | x_v1 -x_v2 |
    //d = | y_v1 -y_v2 |
    auto area = (va.x * vb.y) - (va.y * vb.x);
    if (std::abs (area) < 1)
        return true;
    return false;
}

// 计算从框内点向外运动撞到框壁反弹的位置和向量
void BulletNode::reboundOfRayInRect(const cocos2d::Point &from_pos,
                                    const cocos2d::Vec2 &in_f_vec,
                                    const cocos2d::Rect &in_rect,
                                    cocos2d::Point *out_cross_pos,
                                    cocos2d::Vec2 *out_f_vector)
{
#if 0
    // 使用公式计算, 支持复杂多边形，但是线线相交处碰撞处理又问题，而且计算成本略高
    _posList.clear();
    _posList.push_back(cocos2d::Point(in_rect.origin.x,
                                      in_rect.origin.y));
    _posList.push_back(cocos2d::Point(in_rect.origin.x + in_rect.size.width,
                                      in_rect.origin.y));
    _posList.push_back(cocos2d::Point(in_rect.origin.x + in_rect.size.width,
                                      in_rect.origin.y + in_rect.size.height));
    _posList.push_back(cocos2d::Point(in_rect.origin.x, in_rect.origin.y + in_rect.size.height));
    _posList.push_back(cocos2d::Point(in_rect.origin.x, in_rect.origin.y));
    
    //    auto x = in_rect.origin.x;
    //    auto y = in_rect.origin.y;
    //    auto w = in_rect.size.width;
    //    auto h = in_rect.size.height;
    //    auto wd = in_rect.size.width / 3.0;
    //    auto hd = in_rect.size.height / 3.0;
    //    _posList.push_back(cocos2d::Point(x + wd,       y));
    //    _posList.push_back(cocos2d::Point(x + w - wd,   y));
    //    _posList.push_back(cocos2d::Point(x + w,        y + hd));
    //    _posList.push_back(cocos2d::Point(x + w,        y + h - hd));
    //    _posList.push_back(cocos2d::Point(x + w - wd,   y + h));
    //    _posList.push_back(cocos2d::Point(x + wd,       y + h));
    //    _posList.push_back(cocos2d::Point(x,            y + h - hd));
    //    _posList.push_back(cocos2d::Point(x,            y + hd));
    //    _posList.push_back(cocos2d::Point(x + wd,       y));
    
    auto outPos = posAddVectorLen(from_pos, in_f_vec, maxLenInRect(in_rect));
    
    for (int i = 0; i < _posList.size() - 1; i++) {
        auto seg_start = _posList[i];
        auto seg_end = _posList[i + 1];
        
        if (isPointOnLine(from_pos, seg_start, seg_end)) {
            continue;
        }
        
        float S, T;
        
        if (cocos2d::Vec2::isLineIntersect(from_pos, outPos, seg_start, seg_end, &S, &T )&&
            (S >= 0.0f && S <= 1.0f && T >= 0.0f && T <= 1.0f))
        {
            auto cross = seg_start + T * (seg_end - seg_start);
            auto tmpv = posGetVec(seg_start, seg_end);
            auto f = boundedVec(in_f_vec, tmpv);
            
            if (out_cross_pos) {
                *out_cross_pos = cross;
            }
            
            if (out_f_vector) {
                *out_f_vector = f;
            }
            
            return;
        }
    }
#else
    // 使用向量反向计算，只能计算矩形外框，不需要开方，计算成本低
    auto a = Point(in_rect.origin.x,                        in_rect.origin.y);
    auto b = Point(in_rect.origin.x + in_rect.size.width,   in_rect.origin.y);
    auto c = Point(in_rect.origin.x + in_rect.size.width,   in_rect.origin.y + in_rect.size.height);
    auto d = Point(in_rect.origin.x,                        in_rect.origin.y + in_rect.size.height);
    
    auto outPos = this->posAddVectorLen(from_pos, in_f_vec, this->maxLenInRect(in_rect));
    float S, T;
    
    // ab
    auto seg_start = a;
    auto seg_end = b;
    if (!this->isPointOnLine(from_pos, seg_start, seg_end)) {
        if (cocos2d::Vec2::isLineIntersect(from_pos, outPos, seg_start, seg_end, &S, &T )&&
            (S >= 0.0f && S <= 1.0f && T >= 0.0f && T <= 1.0f)) {
            auto cross = seg_start + T * (seg_end - seg_start);
            
            if (out_cross_pos) {
                *out_cross_pos = cross;
            }
            if (out_f_vector) {
                *out_f_vector = Vec2(in_f_vec.x, -in_f_vec.y);
            }
            return;
        }
    }
    
    // bc
    seg_start = b;
    seg_end = c;
    if (!this->isPointOnLine(from_pos, seg_start, seg_end)) {
        if (cocos2d::Vec2::isLineIntersect(from_pos, outPos, seg_start, seg_end, &S, &T )&&
            (S >= 0.0f && S <= 1.0f && T >= 0.0f && T <= 1.0f)) {
            auto cross = seg_start + T * (seg_end - seg_start);
            
            if (out_cross_pos) {
                *out_cross_pos = cross;
            }
            if (out_f_vector) {
                *out_f_vector = Vec2(-in_f_vec.x, in_f_vec.y);
            }
            return;
        }
    }
    
    // cd
    seg_start = c;
    seg_end = d;
    if (!this->isPointOnLine(from_pos, seg_start, seg_end)) {
        if (cocos2d::Vec2::isLineIntersect(from_pos, outPos, seg_start, seg_end, &S, &T )&&
            (S >= 0.0f && S <= 1.0f && T >= 0.0f && T <= 1.0f)) {
            auto cross = seg_start + T * (seg_end - seg_start);
            
            if (out_cross_pos) {
                *out_cross_pos = cross;
            }
            if (out_f_vector) {
                *out_f_vector = Vec2(in_f_vec.x, -in_f_vec.y);
            }
            return;
        }
    }
    
    // da
    seg_start = d;
    seg_end = a;
    if (!this->isPointOnLine(from_pos, seg_start, seg_end)) {
        if (cocos2d::Vec2::isLineIntersect(from_pos, outPos, seg_start, seg_end, &S, &T )&&
            (S >= 0.0f && S <= 1.0f && T >= 0.0f && T <= 1.0f)) {
            auto cross = seg_start + T * (seg_end - seg_start);
            
            if (out_cross_pos) {
                *out_cross_pos = cross;
            }
            if (out_f_vector) {
                *out_f_vector = Vec2(-in_f_vec.x, in_f_vec.y);
            }
            return;
        }
    }
#endif
    
    assert(false && "计算反弹出错！");
}

