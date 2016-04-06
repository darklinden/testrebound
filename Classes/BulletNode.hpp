#ifndef BulletNode_hpp
#define BulletNode_hpp

#include "cocos2d.h"

using namespace cocos2d;

class BulletNode : public cocos2d::Node
{
public:
    
    virtual bool init(const cocos2d::Point &startPos,
                      const cocos2d::Point &pointPos,
                      const std::vector<std::string> animationFrames);
    
    // implement the "static create()" method manually
    static BulletNode *create(const cocos2d::Point &startPos,
                              const cocos2d::Point &pointPos,
                              const std::vector<std::string> animationFrames);
    
    void playAnimation();
    void runMove(); // 开启子弹动画
    void stopMove(); // 停止子弹动画
    
    CC_SYNTHESIZE(cocos2d::Point, _start_pos, StartPos); // 子弹开始位置
    CC_SYNTHESIZE(cocos2d::Vec2, _start_vector, StartVec); // 子弹发出向量
    
    CC_SYNTHESIZE(cocos2d::Point, _cross_pos, CrossPos); // 子弹开始位置
    CC_SYNTHESIZE(cocos2d::Vec2, _cross_vector, CrossVec); // 子弹结束位置
    
    CC_SYNTHESIZE(float, _move_speed, MoveSpeed); // 速度
    
private:
    
    std::vector<cocos2d::Point> _posList;
    std::vector<std::string> _animationFrames;
    cocos2d::Sprite* _sp = nullptr;
    
    // 修正子弹朝向目标运动方向
    void faceVec(const cocos2d::Vec2& vec);
    
    // 用起点和终点获取向量
    cocos2d::Vec2 posGetVec(const cocos2d::Point& start,
                            const cocos2d::Point& end);
    // 获取边界内最长线段长度
    float maxLenInRect(const cocos2d::Rect& rect);
    
    // 起始点 pos 在向量 vec 上 延伸 len 达到的点, 用于交叉
    cocos2d::Point posAddVectorLen(const cocos2d::Point& pos,
                                   const cocos2d::Vec2& vec,
                                   const float len);
    // 使用向量计算反弹
    cocos2d::Vec2 boundedVec(const cocos2d::Vec2& S, // 物体运动向量
                             const cocos2d::Vec2& B); // 反弹面向量
    
    bool isPointOnLine(const cocos2d::Point& pos,
                       const cocos2d::Point& start,
                       const cocos2d::Point& end);
    
    // 计算从框内点向外运动撞到框壁反弹的位置和向量
    void reboundOfRayInRect(const cocos2d::Point& from_pos,
                            const cocos2d::Vec2& f_vec,
                            const cocos2d::Rect& in_rect,
                            cocos2d::Point* out_cross_pos,
                            cocos2d::Vec2* out_f_vector);
    
    
};

#endif // BulletNode_hpp