/*!
 * @file util.hpp
 * @brief Provides utility functions for tests.
 */

#ifndef SRC_TESTS_PR2018_UTIL_HPP_
#define SRC_TESTS_PR2018_UTIL_HPP_

#define GXL_GEDLIB_SHARED
#include "../../../src/env/ged_env.hpp"

namespace util {

bool is_chemical_dataset(const std::string & dataset) {
	return ((dataset == "AIDS") or (dataset == "Mutagenicity") or (dataset == "acyclic") or (dataset == "alkane") or (dataset == "mao") or (dataset == "pah"));
}

bool is_letter_dataset(const std::string & dataset) {
	return ((dataset == "Letter_HIGH") or (dataset == "Letter_LOW") or (dataset == "Letter_MED"));
}

void check_dataset(const std::string & dataset) {
	if (not (is_chemical_dataset(dataset) or is_letter_dataset(dataset) or (dataset == "CMU-GED") or (dataset == "Fingerprint") or (dataset == "GREC") or (dataset == "Protein"))) {
		throw ged::Error(std::string("Dataset \"") + dataset + "\" does not exists.");
	}
}

std::string graph_dir(const std::string & dataset) {
	std::string root_dir("../../../data/datasets/");
	if ((dataset == "AIDS") or (dataset == "Fingerprint") or (dataset == "GREC") or (dataset == "Protein") or (dataset == "Mutagenicity")) {
		return (root_dir + dataset + "/data/");
	}
	else if ((dataset == "Letter_HIGH")) {
		return (root_dir + "Letter/HIGH/");
	}
	else if ((dataset == "Letter_LOW")) {
		return (root_dir + "Letter/LOW/");
	}
	else if ((dataset == "Letter_MED")) {
		return (root_dir + "Letter/MED/");
	}
	else if (dataset == "CMU-GED") {
		return (root_dir + dataset + "/CMU/");
	}
	else if ((dataset == "acyclic") or (dataset == "alkane") or (dataset == "mao") or (dataset == "pah")) {
		return (root_dir + dataset + "/");
	}
	else {
		throw ged::Error(std::string("Dataset \"") + dataset + "\" does not exists.");
	}
	return "";
}

std::string train_collection(const std::string & dataset) {
	std::string root_dir("../collections/");
	check_dataset(dataset);
	if (is_letter_dataset(dataset)) {
		return (root_dir + "Letter_50.xml");
	}
	return root_dir + dataset + "_50.xml";
}

std::string test_collection(const std::string & dataset) {
	std::string root_dir("../../../data/collections/");
	check_dataset(dataset);
	if (is_letter_dataset(dataset)) {
		return (root_dir + "Letter.xml");
	}
	return root_dir + dataset + ".xml";
}

std::string config_prefix(const std::string & dataset) {
	check_dataset(dataset);
	return std::string("../output/" + dataset + "_");
}

std::string init_options(const std::string & dataset, const std::string & config_suffix, const std::string & data_suffix = "", bool save_train = false, bool load_train = false, std::size_t threads = 8) {
	check_dataset(dataset);
	std::string options("--threads ");
	options += std::to_string(threads) + " --save ../output/";
	options += dataset + "_" + config_suffix + ".ini";
	if (save_train) {
		if (load_train) {
			throw ged::Error("Training data cannot be both saved and loaded.");
		}
		options += " --save-train ../output/" + dataset + "_" + data_suffix + ".data";
	}
	if (load_train) {
		options += " --load-train ../output/" + dataset + "_" + data_suffix + ".data";
	}
	return options;
}

std::string ground_truth_option(const std::string & dataset) {
	check_dataset(dataset);
	//if (is_letter_dataset(dataset)) {
	//	return std::string(" --ground-truth-method EXACT");
	//}
	return std::string(" --ground-truth-method IPFP");
}

ged::Options::EditCosts edit_costs(const std::string & dataset) {
	if (is_chemical_dataset(dataset)) {
		return ged::Options::EditCosts::CHEM_2;
	}
	else if (is_letter_dataset(dataset)) {
		return ged::Options::EditCosts::LETTER;
	}
	else if (dataset == "CMU-GED") {
		return ged::Options::EditCosts::CMU;
	}
	else if (dataset == "Fingerprint") {
		return ged::Options::EditCosts::FINGERPRINT;
	}
	else if (dataset == "GREC") {
		return ged::Options::EditCosts::GREC_2;
	}
	else if (dataset == "Protein") {
		return ged::Options::EditCosts::PROTEIN;
	}
	else {
		throw ged::Error(std::string("Dataset \"") + dataset + "\" does not exists.");
	}
	return ged::Options::EditCosts::CONSTANT;
}

ged::Options::GXLNodeEdgeType node_type(const std::string & dataset) {
	check_dataset(dataset);
	if ((dataset == "Fingerprint") or (dataset == "CMU-GED")) {
		return ged::Options::GXLNodeEdgeType::UNLABELED;
	}
	return ged::Options::GXLNodeEdgeType::LABELED;
}

ged::Options::GXLNodeEdgeType edge_type(const std::string & dataset) {
	check_dataset(dataset);
	if (is_letter_dataset(dataset)) {
		return ged::Options::GXLNodeEdgeType::UNLABELED;
	}
	return ged::Options::GXLNodeEdgeType::LABELED;
}

std::unordered_set<std::string> irrelevant_node_attributes(const std::string & dataset) {
	check_dataset(dataset);
	std::unordered_set<std::string> irrelevant_attributes;
	if ((dataset == "AIDS")) {
		irrelevant_attributes.insert({"x", "y", "symbol"});
	}
	else if (dataset == "Protein") {
		irrelevant_attributes.insert("aaLength");
	}
	return irrelevant_attributes;
}

std::unordered_set<std::string> irrelevant_edge_attributes(const std::string & dataset) {
	check_dataset(dataset);
	std::unordered_set<std::string> irrelevant_attributes;
	if ((dataset == "GREC")) {
		irrelevant_attributes.insert({"angle0", "angle1"});
	}
	else if (dataset == "Protein") {
		irrelevant_attributes.insert({"distance0", "distance1"});
	}
	else if (dataset == "Fingerprint") {
		irrelevant_attributes.insert("angle");
	}
	return irrelevant_attributes;
}

ged::Options::InitType init_type(const std::string & dataset) {
	if (is_chemical_dataset(dataset) or (dataset == "Protein")) {
		return ged::Options::InitType::EAGER_WITHOUT_SHUFFLED_COPIES;
	}
	return ged::Options::InitType::LAZY_WITHOUT_SHUFFLED_COPIES;
}

void setup_environment(const std::string & dataset, bool train, ged::GEDEnv<ged::GXLNodeID, ged::GXLLabel, ged::GXLLabel> & env) {
	env.load_gxl_graphs(graph_dir(dataset), (train ? train_collection(dataset) : test_collection(dataset)), node_type(dataset), edge_type(dataset), irrelevant_node_attributes(dataset), irrelevant_edge_attributes(dataset));
	env.set_edit_costs(edit_costs(dataset));
	env.init(init_type(dataset));
}

void setup_datasets(std::vector<std::string> & datasets) {
	datasets = {"Letter_HIGH", "pah", "AIDS", "Protein", "GREC", "Fingerprint"};
}

}

#endif /* SRC_TESTS_PR2018_UTIL_HPP_ */