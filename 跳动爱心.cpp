#include <easyx.h>
#include <conio.h>
#include <ctime>
#include <math.h>
#include <stdlib.h>

// 定义点结构体，包含坐标和颜色
struct Point {
    double x, y;
    COLORREF color;
};

// 霓虹紫色系颜色数组
COLORREF colors[256] = {
    RGB(255,105,180), // 热粉色
    RGB(218,112,214), // 紫红色
    RGB(255,182,193), // 浅粉色
    RGB(238,130,238), // 紫罗兰色
    RGB(255,20,147),  // 深粉色
    RGB(199,21,133),  // 中等紫罗兰红色
    RGB(255,160,122)  // 浅鲑鱼色（增加层次感）
};

// 屏幕尺寸
const int xScreen = 1200;
const int yScreen = 800;

// 数学常数
const double PI = 3.1415926535;
const double e = 2.71828;

// 粒子系统参数
const double average_distance = 0.162; // 粒子平均间距
const int quantity = 506;  // 基础粒子数量
const int circles = 210;   // 扩散圆数
const int frames = 20;     // 动画帧数

// 存储点集的数组
Point origin_points[quantity];      // 原始心形点集
Point points[circles * quantity];   // 动态粒子点集
IMAGE images[frames];               // 动画帧缓存区

// 坐标系统：数学坐标->屏幕坐标
double screen_x(double x) {
    return x + xScreen / 2; // 将原点移到屏幕中心
}

double screen_y(double y) {
    return -y + yScreen / 2; // y轴翻转
}

// 生成指定范围的随机数
int create_random(int x1, int x2) {
    if (x2 > x1) return rand() % (x2 - x1 + 1) + x1;
    return x1; // 处理x2<=x1的情况，避免未定义行为
}

// 创建粒子数据并生成动画帧
void create_data() {
    // 生成原始心形点集（参数方程：笛卡尔心形曲线）
    int index = 0;
    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    for (double radian = 0.1; radian <= 2 * PI; radian += 0.005) {
        x2 = 16 * pow(sin(radian), 3);
        y2 = 13 * cos(radian) - 5 * cos(2 * radian) - 2 * cos(3 * radian) - cos(4 * radian);

        // 根据间距筛选点
        double distance = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
        if (distance > average_distance) {
            x1 = x2, y1 = y2;
            origin_points[index].x = x2;
            origin_points[index].y = y2;
            index++;
        }
    }

    // 生成动态粒子（多层扩散效果）
    index = 0;
    for (double size = 0.1, Tightness = 1.4; size <= 20; size += 0.1) {
        double success_p = 1 / (1 + pow(e, 8 - size / 2)); // 概率衰减曲线
        if (Tightness > 1) Tightness -= 0.0025; // 颜色亮度渐变
        for (int i = 0; i < quantity; ++i) {
            if (success_p > create_random(0, 100) / 100.0) {
                COLORREF color = colors[create_random(0, 6)];
                // 颜色亮度调整
                points[index].color = RGB(
                    GetRValue(color) / Tightness,
                    GetGValue(color) / Tightness,
                    GetBValue(color) / Tightness
                );
                // 添加随机扰动
                points[index].x = size * origin_points[i].x + create_random(-4, 4);
                points[index++].y = size * origin_points[i].y + create_random(-4, 4);
            }
        }
    }

    int points_size = index;

    // 生成各帧动画
    for (int frame = 0; frame < frames; ++frame) {
        images[frame] = IMAGE(xScreen, yScreen);
        SetWorkingImage(&images[frame]);
        cleardevice(); // 清空画面

        // 更新粒子位置并绘制
        for (index = 0; index < points_size; ++index) {
            // 计算粒子运动（抛物线速度）
            double x = points[index].x, y = points[index].y;
            double distance = sqrt(x * x + y * y);
            double distance_increase = -0.0009 * distance * distance + 0.35714 * distance;

            double x_increase = distance_increase * x / distance / frames;
            double y_increase = distance_increase * y / distance / frames;

            points[index].x += x_increase;
            points[index].y += y_increase;

            // 绘制粒子
            setfillcolor(points[index].color);
            solidcircle((int)screen_x(points[index].x), (int)screen_y(points[index].y), 1);
        }

        // 添加随机闪烁粒子
        for (double size = 17; size < 23; size += 0.3) {
            for (index = 0; index < quantity; ++index) {
                if ((create_random(0, 100) / 100.0 > 0.6 && size >= 20) ||
                    (size < 20 && create_random(0, 100) / 100.0 > 0.95)) {
                    double x, y;
                    // 大粒子添加更大幅度扰动
                    if (size >= 20) {
                        x = origin_points[index].x * size + create_random(-frame * frame / 5 - 15, frame * frame / 5 + 15);
                        y = origin_points[index].y * size + create_random(-frame * frame / 5 - 15, frame * frame / 5 + 15);
                    }
                    else {
                        x = origin_points[index].x * size + create_random(-5, 5);
                        y = origin_points[index].y * size + create_random(-5, 5);
                    }
                    setfillcolor(colors[create_random(0, 6)]);
                    solidcircle((int)screen_x(x), (int)screen_y(y), 1);
                }
            }
        }
    }
    SetWorkingImage(); // 恢复默认绘图目标
}

int main() {
    initgraph(xScreen, yScreen); // 初始化图形窗口
    BeginBatchDraw(); // 开始批量绘制
    srand((unsigned)time(0)); // 随机数种子

    create_data(); // 生成所有动画数据

    // 动画循环（呼吸效果）
    bool extend = true, shrink = false;
    for (int frame = 0; !_kbhit();) { // 按任意键退出
        putimage(0, 0, &images[frame]); // 显示当前帧
        FlushBatchDraw(); // 刷新绘制
        Sleep(20); // 控制帧率
        cleardevice(); // 清空画面

        // 更新帧序号（先扩展后收缩）
        if (extend) {
            if (frame == 19) { shrink = true; extend = false; }
            else ++frame;
        }
        else {
            if (frame == 0) { shrink = false; extend = true; }
            else --frame;
        }
    }

    EndBatchDraw(); // 结束批量绘制
    closegraph(); // 关闭图形窗口
    return 0;
}