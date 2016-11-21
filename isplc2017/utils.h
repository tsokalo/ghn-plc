/*
 * utils.h
 *
 *  Created on: 04.11.2016
 *      Author: tsokalo
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include <cmath>
#include <boost/math/special_functions/binomial.hpp>

typedef double duration_t;
struct timings_t {
	/*
	 * duration of payload of the physical layer data unit / us
	 */
	duration_t packet_duration;
	/*
	 * preamble duration / us
	 */
	duration_t preamble;
	/*
	 * header on physical layer / us
	 */
	duration_t phy_header;
	/*
	 * propagation delay / us
	 */
	duration_t tau;
	/*
	 * time slot duration / us
	 */
	duration_t slot;
	/*
	 * inter-frame guard interval
	 */
	duration_t guard;
	/*
	 * guard interval between the data and acknowledgment frames
	 */
	duration_t tiafg;
	/*
	 * acknowledgment frame duration
	 */
	duration_t ack_frame;
	/*
	 * channel sensing duration
	 */
	duration_t cca;
	/*
	 * coding rate
	 */
	double c;
};

/*
 * collision probability if all n nodes use the maximum contention window size equal cw
 */
double p_c(uint16_t cw, uint16_t n) {

	auto sub = [](uint16_t cw, uint16_t n, uint16_t i)->long double
	{
		long double v = 0;
		for(uint16_t j = 2; j <= n; j++)
		{
			v += boost::math::binomial_coefficient<long double>(n, j) * pow(cw - i, n - j);
		}
		return v;
	};;

	long double cwn = 1 / pow(cw, n);

	long double v = 0;
	for (uint16_t i = 1; i <= cw - 1; i++) {
		v += sub(cw, n, i);
	}
	v = v * cwn + cwn;

	return v;
}
/*
 * expectation value of the minimum backoff counter when n nodes contend for the channel access
 * and use the same contention window size equal cw
 *
 * unit [number of time slots]
 */
double b_e(uint16_t cw, uint16_t n) {

	auto div_int = [](uint16_t u, uint16_t v)->double
	{
		return (double)u / (double)v;
	};;

	long double v = 0;
	for (uint16_t i = 1; i <= cw; i++) {
		v += i * (pow(1 - div_int(i - 1, cw), n) - pow(1 - div_int(i, cw), n));
	}
	return v;
}
/*
 * MAC layer efficiency when using CSMA/CA
 */
double mac_eff_ca(uint16_t cw, uint16_t n, timings_t t) {

	double t1 = t.tau + t.guard + t.preamble + t.phy_header + t.packet_duration
			/ t.c + t.slot * b_e(cw, n) + t.cca;
	double t2 = t.tau + t.tiafg + t.ack_frame;
	return (t.packet_duration / (t1 / (1 - p_c(cw, n)) + t2));
}
/*
 * MAC layer efficiency when using CSMA/CD
 */
double mac_eff_cd(uint16_t cw, uint16_t n, timings_t t) {

	double t1 = t.tau + t.guard + t.preamble + t.slot * b_e(cw, n) + t.cca;
	double t2 = t.tau + t.tiafg + t.ack_frame + t.phy_header
			+ t.packet_duration / t.c;
	return (t.packet_duration / (t1 / (1 - p_c(cw, n)) + t2));
}

#endif /* UTILS_H_ */
