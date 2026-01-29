/**
 * @file util.h
 * @author Kwon Jinwoo (jinwoo@xslab.co.kr)
 * @brief Extra Util
 * @date 2025-09-04
 */

#ifndef INCLUDE_UTIL
#define INCLUDE_UTIL

/**
 * @brief 지정된 입력을 지정된 범위로 변환합니다
 * 
 * @param[in]	x			입력 값입니다
 * @param[in]	in_min		입력 최소 값입니다
 * @param[in]	in_max		입력 최대 값입니다
 * @param[in]	out_min		출력 최소 값입니다
 * @param[in]	out_max		출력 최대 값입니다
 * @return 변환 값입니다
 */
long map(long x, long in_min, long in_max, long out_min, long out_max);

#endif /* INCLUDE_UTIL */
