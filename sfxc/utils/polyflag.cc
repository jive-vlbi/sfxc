#include <iostream>
#include <fstream>
#include <cmath>
#include <set>

#include <unistd.h>

#include <json/json.h>
#include <vex/Vex++.h>

#include "utils.h"
#include "delay_table_akima.h"
#include "correlator_time.h"

double
sgn(double x)
{
	if (x > 0)
		return 1;
	if (x < 0)
		return -1;
	return 0;
}

void
print_interval(std::string& station1, std::string& station2, Time& start, Time& stop)
{
	struct tm tm;
	char sec_buf[64];
	char csec_buf[64];

	strptime(start.date_string().c_str(), "%Yy%jd%Hh%Mm%Ss", &tm);
	strftime(sec_buf, sizeof(sec_buf), "%Y/%m/%d/%H:%M:%S", &tm);
	snprintf(csec_buf, sizeof(csec_buf), ".%02d", (int)((uint64_t)start.get_time_usec() % 1000000) / 10000);
	std::cout << station1 << "-" << station2 << "\t" << sec_buf << csec_buf;
	strptime(stop.date_string().c_str(), "%Yy%jd%Hh%Mm%Ss", &tm);
	strftime(sec_buf, sizeof(sec_buf), "%Y/%m/%d/%H:%M:%S", &tm);
	snprintf(csec_buf, sizeof(csec_buf), ".%02d", (int)((uint64_t)stop.get_time_usec() % 1000000) / 10000);
	std::cout << "-" << sec_buf << csec_buf << std::endl;
}

void
usage()
{
	std::cout << "usage: polyflag [-i N] [-w N] [-x STATION] VEXFILE CTRLFILE ..."  << std::endl;
	exit(1);
}

int
main(int argc, char *argv[])
{
	double min_wrap = 4.0;
	Time t_int = Time(1e6);
	bool t_int_set = false;
	std::set<std::string> excluded_stations;
	int ch;

	while ((ch = getopt(argc, argv, "i:w:x:")) != -1) {
		switch (ch) {
		case 'i':
			t_int = Time(atof(optarg) * 1e6);
			t_int_set = true;
			break;
		case 'w':
			min_wrap = atof(optarg);
			break;
		case 'x':
			excluded_stations.insert(optarg);
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 2)
		usage();

	Vex vex;
	vex.open(argv[0]);

	for (int arg = 1; arg < argc; arg++) {
		Json::Value ctrl;
		Json::Reader reader;
		std::ifstream in(argv[arg]);
		if (!reader.parse(in, ctrl)) {
			std::cout << reader.getFormatedErrorMessages() << std::endl;
			exit(1);
		}

		std::string exper = vex.get_root_node()["GLOBAL"]["EXPER"]->to_string();
		std::string exper_name = vex.get_root_node()["EXPER"][exper]["exper_name"]->to_string();
		Time start_date(vex.get_root_node()["EXPER"][exper]["exper_nominal_start"]->to_string());

		std::string delay_directory = ctrl["delay_directory"].asString().substr(7);
		Time start_time_ctrl(ctrl["start"].asString());
		Time stop_time_ctrl(ctrl["stop"].asString());

		if (!t_int_set)
			t_int = Time(ctrl["integr_time"].asDouble() * 1e6);

		std::vector<std::string> station_names;
		std::vector<Delay_table> delay_tables;
		for (Json::Value::iterator station = ctrl["stations"].begin();
		     station != ctrl["stations"].end(); station++) {
			if (excluded_stations.count((*station).asString()) > 0)
				continue;
			station_names.push_back((*station).asString());

			std::string delay_table_name;
			delay_table_name = delay_directory + "/" + exper_name + "_" + (*station).asString() + ".del";

			Delay_table delay_table;
			delay_table.open(delay_table_name.c_str());
			delay_tables.push_back(delay_table);
		}

		for (Vex::Node::const_iterator scan = vex.get_root_node()["SCHED"]->begin();
		     scan != vex.get_root_node()["SCHED"]->end(); scan++) {
			Time start_time_scan(scan["start"]->to_string());
			int len = 0;
			for (Vex::Node::const_iterator station = scan->begin("station");
			     station != scan->end("station"); station++)
				len = std::max(len, station[2]->to_int_amount("sec"));
			Time stop_time_scan = start_time_scan + Time(len * 1e6);

			if (stop_time_scan <= start_time_ctrl || start_time_scan >= stop_time_ctrl)
				continue;
                        std::vector<Delay_table_akima> akima(station_names.size());
                        Time dt = stop_time_scan - start_time_scan;
			for (int i = 0; i < station_names.size(); i++) {
				akima[i] = delay_tables[i].create_akima_spline(start_time_scan, dt);
                        } 
			std::string mode = scan["mode"]->to_string();
			std::string freq = vex.get_root_node()["MODE"][mode]["FREQ"]->begin()->to_string();

			double min_freq = HUGE_VAL;
			for (Vex::Node::const_iterator chan_def = vex.get_root_node()["FREQ"][freq]->begin("chan_def");
			     chan_def != vex.get_root_node()["FREQ"][freq]->end("chan_def"); chan_def++)
				min_freq = std::min(min_freq, chan_def[1]->to_double_amount("MHz") * 1e6);

			for (int i = 0; i < station_names.size(); i++) {
				while (delay_tables[i].start_time_scan() < start_time_scan) {
					if (!delay_tables[i].initialise_next_scan())
						break;
				}

				for (int j = i + 1; j < station_names.size(); j++) {
					bool start_time_set = false;
					Time start_time, stop_time;

					for (Time time = start_time_scan; time < (stop_time_scan - t_int); time += t_int) {
#if 0
						double rate1 = (delay_tables[i].rate(time) - delay_tables[j].rate(time));
						double rate2 = (delay_tables[i].rate(time + t_int) - delay_tables[j].rate(time + t_int));
						if (sgn(rate1) != sgn(rate2)) {
							std::cout << station_names[i] << "-" << station_names[j] << "\t"
								  << time.date_string() << "\t" << rate1 << "\t" << rate2 << std::endl;
						}
#endif
						double rate = (akima[i].rate(time + (t_int / 2)) - akima[j].rate(time + (t_int / 2)));
						if (std::abs(rate) < (min_wrap / (t_int.get_time() * min_freq))) {
							if (!start_time_set) {
								start_time = time;
								start_time_set = true;
							}
						} else {
							if (start_time_set) {
								stop_time = time;
								print_interval(station_names[i], station_names[j], start_time, stop_time);
								start_time_set = false;
							}
						}
					}

					if (start_time_set) {
						stop_time = stop_time_scan;
						print_interval(station_names[i], station_names[j], start_time, stop_time);
					}
				}
			}
		}
	}

	return 0;
}
