/*
 * main.cpp
 *
 *  Created on: 04.11.2016
 *      Author: tsokalo
 */

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <memory>

#include "utils.h"

int main(int argc, char *argv[]) {
	std::cout << "Program start" << std::endl;

	timings_t t;
	t.preamble = 51.2;
	t.phy_header = 42.24; // 42.24 ... 42.24 * 3
	t.tau = 0.005; // 10 meter copper cable
	t.slot = 35.84;
	t.guard = 90;
	t.tiafg = 122.88;
	t.ack_frame = 102.4;
	t.cca = 84.48;
	t.c = 20.0 / 21.0;

	/*
	 * OFDM symbol duration / us
	 */
	duration_t symbol = 42.24;
	duration_t min_packet_duration = symbol;
	duration_t max_packet_duration = floor((40000 - t.preamble - t.phy_header
			- t.guard) / symbol) * symbol;
	duration_t middle_packet_duration = (max_packet_duration
			- min_packet_duration) / 2;

	uint16_t max_cw = 200;
	uint16_t max_n = 10;

	auto calc_a = [&]()
	{
		uint16_t cw_ca_opt = 0, cw_cd_opt = 0;
		double ef_ca_opt = 0, ef_cd_opt = 0;
		for (uint16_t n = 2; n <= max_n; n++) {
			for (uint16_t cw = 2; cw <= max_cw; cw++) {

				std::cout << t.packet_duration << "\t" << n << "\t" << cw << "\t" << mac_eff_ca(cw, n, t)
				<< "\t" << mac_eff_cd(cw, n, t) << std::endl;
			}
		}
	};;

	auto calc_b = [&]()
	{
		for (uint16_t n = 2; n <= max_n; n++) {

			uint16_t cw_ca_opt = 0, cw_cd_opt = 0;
			double ef_ca_opt = 0, ef_cd_opt = 0;

			for (uint16_t cw = 2; cw <= max_cw; cw++) {

				double ef_ca = mac_eff_ca(cw, n, t);
				if(ef_ca > ef_ca_opt)
				{
					ef_ca_opt = ef_ca;
					cw_ca_opt = cw;
				}
				double ef_cd = mac_eff_cd(cw, n, t);
				if(ef_cd > ef_cd_opt)
				{
					ef_cd_opt = ef_cd;
					cw_cd_opt = cw;
				}
			}
			std::cout << t.packet_duration / max_packet_duration << "\t" << n << "\t" << ef_ca_opt
			<< "\t" << cw_ca_opt << "\t" << ef_cd_opt
			<< "\t" << cw_cd_opt << std::endl;
		}
	};;

	auto calc_c = [&]()
	{
		uint16_t cw_ca_opt = 0, cw_cd_opt = 0;
		double ef_ca_opt = 0, ef_cd_opt = 0;
		uint16_t n = 6;

		for (uint16_t cw = 2; cw <= max_cw; cw++) {

			std::cout << t.packet_duration << "\t" << n << "\t" << cw << "\t" << mac_eff_ca(cw, n, t)
			<< "\t" << mac_eff_cd(cw, n, t) << std::endl;
		}
	};;

	auto calc_d = [&]()
	{
		typedef uint16_t csma_t;// 0 = CA, 4 = CD
		typedef uint16_t num_senders_t;
		typedef uint16_t cw_t;

		struct cset
		{
			cw_t cw; // contention window size
			num_senders_t n; // number of senders
			csma_t csma; // collision avoidance or collision detection
			double tp; // average duration of the MAC PDU
		};
		std::vector<cset> cv;

		assert(argc > 1);
		std::string path_stat = argv[1];

		std::ifstream f (path_stat, std::ios_base::in);
		std::string line;
		std::vector<std::string> lines;

		while (getline (f, line, '\n'))
		{
			lines.push_back(line);
			std::stringstream ss (line);

			cset s;
			ss >> s.csma;
			ss >> s.n;
			ss >> s.cw;
			double v;
			ss >> v;
			ss >> v;
			ss >> v;
			ss >> v;
			ss >> v;
			ss >> s.tp;

			cv.push_back(s);
		}
		f.close ();

		assert(lines.size() == cv.size());

		std::string path = argv[0]; // get path from argument 0
		path = path.substr (0, path.rfind ("/") + 1);
		std::ofstream fo (path + "data.txt", std::ios_base::out);

		for(uint16_t i = 0; i < cv.size(); i++)
		{
			t.packet_duration = cv.at(i).tp;
			double eff = (cv.at(i).csma == 0) ? mac_eff_ca(cv.at(i).cw, cv.at(i).n, t) : mac_eff_cd(cv.at(i).cw, cv.at(i).n, t);
			fo << lines.at(i) << "\t" << eff << std::endl;
			std::cout << cv.at(i).csma << "\t" << cv.at(i).n << "\t" << cv.at(i).cw << "\t" << eff << std::endl;
		}
		fo.close ();
	};;

//		t.packet_duration = min_packet_duration;
//		calc_a();
//		t.packet_duration = middle_packet_duration;
//		calc_a();
//		t.packet_duration = max_packet_duration;
//		calc_a();

		for (uint16_t c = 1; c <= 5; c++) {
			t.packet_duration = max_packet_duration / 5.0 * (double) c;
			calc_b();
		}

	//	t.packet_duration = 3190;
	//	calc_c();

//	calc_d();

	std::cout << "Finished successfully" << std::endl;
	return 0;
}

