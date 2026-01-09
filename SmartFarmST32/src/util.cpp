/**
 * @file util.cpp
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief Extra Util
 * @date 2025-09-04
 */

#include <util.h>

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}