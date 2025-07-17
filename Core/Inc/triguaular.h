#ifndef __TRIGUAULAR_H__
#define __TRIGUAULAR_H__


#define MAX_SEGMENTS 600 // 支持最多150个周期（每周期4个点）
typedef struct {
    float slope;     // 斜率 (a)
    float intercept; // 截距 (b)
    int start_idx;    // 起始索引
    int end_idx;      // 结束索引
} LinearSegment;

typedef struct {
    float x;         // x坐标
    float y;         // y坐标
} PreciseTurningPoint;

typedef struct {
    float peak;          // 峰值y值
    float valley;        // 谷值y值
    float frequency;     // 频率 (1/Δx)
    float period;        // 周期 (Δx)
    float rise_time;     // 上升时间 (Δx)
    float fall_time;     // 下降时间 (Δx)
    float symmetry;      // 对称性 (%)
    int cycles;           // 检测到的周期数
} TriangleWaveAnalysis;

void linear_regression(const float* x, const float* y, int start, int end,
                      float* slope, float* intercept);
PreciseTurningPoint calculate_intersection(const LinearSegment* seg1,
                                          const LinearSegment* seg2,
                                          const float* x) ;
TriangleWaveAnalysis analyze_triangle_wave(const float* x, const float* y, uint16_t n);
void Genery_X(float *x, uint16_t len,uint32_t fre);

#endif /* __TRIGUAULAR_H__ */

