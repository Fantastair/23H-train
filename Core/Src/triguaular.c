#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "triguaular.h"
#define MAX_SEGMENTS 600 // 支持最多150个周期（每周期4个点）




void linear_regression(const float* x, const float* y, int start, int end,
                      float* slope, float* intercept) {
    float sum_x = 0.0, sum_y = 0.0;
    float sum_xy = 0.0, sum_xx = 0.0;
    int n = end - start + 1;

    for (int i = start; i <= end; i++) {
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_xx += x[i] * x[i];
    }

    float denom = n * sum_xx - sum_x * sum_x;
    if (fabs(denom) < 1e-10) {
        *slope = (n > 1) ? (y[end] - y[start]) / (x[end] - x[start]) : 0.0;
        *intercept = (n > 1) ? y[start] - (*slope) * x[start] : y[start];
    } else {
        *slope = (n * sum_xy - sum_x * sum_y) / denom;
        *intercept = (sum_y - (*slope) * sum_x) / n;
    }
}

PreciseTurningPoint calculate_intersection(const LinearSegment* seg1,
                                          const LinearSegment* seg2,
                                          const float* x) {  // 添加x数组参数
    PreciseTurningPoint point = {0};
    
    // 处理垂直线段或平行线段
    if (fabs(seg1->slope - seg2->slope) < 1e-10) {
        // 取中点作为交点 - 使用实际的x值
        float x1 = x[seg1->end_idx];
        float x2 = x[seg2->start_idx];
        point.x = (x1 + x2) / 2.0;
        point.y = (seg1->slope * point.x + seg1->intercept + 
                  seg2->slope * point.x + seg2->intercept) / 2.0;
    } else {
        point.x = (seg2->intercept - seg1->intercept) / (seg1->slope - seg2->slope);
        point.y = seg1->slope * point.x + seg1->intercept;
    }
    return point;
}

TriangleWaveAnalysis analyze_triangle_wave(const float* x, const float* y, uint16_t n) {
    TriangleWaveAnalysis result = {0};
    if (n < 10) return result;  // 数据点不足
    
    uint16_t indices[MAX_SEGMENTS];  // 线段端点索引数组
    uint16_t num_indices = 0;        // 有效索引数量
    
    // 1. 检测第一个下降线起点 (起始波峰)
    for (uint16_t i = 2; i < n - 1; i++) {
        if (y[i] > y[i-2] && y[i] > y[i+1]) {
            indices[num_indices++] = i;
            break;
        }
    }
    if (num_indices == 0) return result;  // 未找到起始点
    
    // 2. 检测最后一个上升线终点 (结束波峰)
    uint16_t end_index = 0;
    for (uint16_t i = n - 2; i > 1; i--) {
        if (y[i] > y[i+2] && y[i] > y[i-1]) {
            end_index = i;
            break;
        }
    }
    if (end_index == 0 || end_index <= indices[0]) return result;  // 无效结束点
    
    // 3. 填充中间点 (严格遵循您的检测条件)
    for (uint16_t i = indices[0] + 1; i < end_index; i++) {
        // 边界检查 (确保i-2和i+2不越界)
        if (i < 2 || i >= n - 2) continue;
        
        // 上升线起点检测 (波谷位置) - 取 i+1
        if ((y[i] > y[i+2] && y[i] > y[i-1])) {
            if (num_indices < MAX_SEGMENTS) {
                indices[num_indices++] = i + 1;
            }
        }
        
        // 下降线起点检测 (波峰位置) - 取 i+1
        if ((y[i] < y[i+2] && y[i] < y[i-1])) {
            if (num_indices < MAX_SEGMENTS) {
                indices[num_indices++] = i + 1;
            }
        }
        
        // 下降线终点检测 (波谷位置) - 取 i
        if ((y[i] < y[i+2] && y[i] < y[i-1])) {
            if (num_indices < MAX_SEGMENTS) {
                indices[num_indices++] = i;
            }
        }
        
        // 上升线终点检测 (波峰位置) - 取 i
        if ((y[i] > y[i+2] && y[i] > y[i-1])) {
            if (num_indices < MAX_SEGMENTS) {
                indices[num_indices++] = i;
            }
        }
    }
    
    // 添加结束点
    if (num_indices < MAX_SEGMENTS) {
        indices[num_indices++] = end_index;
    }
    
    // 4. 检查是否有足够线段
    if (num_indices < 4) return result;  // 至少需要一个完整周期
    
    // 5. 为每个线段进行线性回归
    uint16_t segment_count = num_indices - 1;
    LinearSegment segments[MAX_SEGMENTS];
    
    for (uint16_t i = 0; i < segment_count; i++) {
        linear_regression(x, y, indices[i], indices[i+1],
                         &segments[i].slope, &segments[i].intercept);
        segments[i].start_idx = indices[i];
        segments[i].end_idx = indices[i+1];
    }
    
    // 6. 计算线段交点 (精确转折点)
    PreciseTurningPoint turning_points[MAX_SEGMENTS];
    uint16_t point_count = segment_count - 1;
    
    for (uint16_t i = 0; i < point_count; i++) {
        // 传递x数组给calculate_intersection
        turning_points[i] = calculate_intersection(&segments[i], &segments[i+1], x);
    }
    
    // 7. 分类转折点并计算参数
    float peak_sum = 0.0, valley_sum = 0.0;
    float rise_time_sum = 0.0, fall_time_sum = 0.0;
    uint16_t peak_count = 0, valley_count = 0;
    uint16_t valid_cycles = 0;
    
    // 根据线段斜率方向分类交点
    for (uint16_t i = 0; i < point_count; i++) {
        // 下降线→上升线交点 = 波谷
        if (segments[i].slope < 0 && segments[i+1].slope > 0) {
            valley_sum += turning_points[i].y;
            valley_count++;
            
            // 计算上升时间 (当前波谷到下一个波峰)
            if (i + 1 < point_count) {
                rise_time_sum += turning_points[i+1].x - turning_points[i].x;
            }
        }
        // 上升线→下降线交点 = 波峰
        else if (segments[i].slope > 0 && segments[i+1].slope < 0) {
            peak_sum += turning_points[i].y;
            peak_count++;
            
            // 计算下降时间 (当前波峰到下一个波谷)
            if (i + 1 < point_count) {
                fall_time_sum += turning_points[i+1].x - turning_points[i].x;
            }
        }
    }
    
    // 8. 计算结果
    valid_cycles = (peak_count < valley_count) ? peak_count : valley_count;
    result.cycles = valid_cycles;
    
    if (valid_cycles > 0) {
        // 峰值和谷值
        if (peak_count > 0) result.peak = peak_sum / peak_count;
        if (valley_count > 0) result.valley = valley_sum / valley_count;
        
        // 上升和下降时间
        if (peak_count > 0 && valley_count > 0) {
            result.rise_time = rise_time_sum / valid_cycles;
            result.fall_time = fall_time_sum / valid_cycles;
        }
        
        // 周期和频率
        if (peak_count > 1) {
            float total_period = turning_points[point_count-1].x - turning_points[0].x;
            result.period = total_period / valid_cycles;
            result.frequency = 1.0 / result.period;
        }
        
        // 对称性
        if (result.fall_time > 0) {
            result.symmetry = (result.rise_time / result.fall_time) * 100.0;
        }
    }
    
    return result;
}

void Genery_X(float *x, uint16_t len,uint32_t fre){
	for(uint16_t i=0;i<len;i++){
		x[i]=i/fre;
	}
}