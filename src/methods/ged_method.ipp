/*!
 * @file  ged_method.ipp
 * @brief GEDMethod class definition
 */

#ifndef SRC_METHODS_GED_METHOD_IPP_
#define SRC_METHODS_GED_METHOD_IPP_

namespace ged {

// === Definitions of destructors and constructors. ===
template<class UserNodeLabel, class UserEdgeLabel>
GEDMethod<UserNodeLabel, UserEdgeLabel>::
~GEDMethod() {}

template<class UserNodeLabel, class UserEdgeLabel>
GEDMethod<UserNodeLabel, UserEdgeLabel>::
GEDMethod(const GEDData<UserNodeLabel, UserEdgeLabel> & ged_data) :
initialized_{false},
ged_data_(ged_data),
options_(),
lower_bound_(0.0),
upper_bound_(std::numeric_limits<double>::infinity()),
matching_(),
runtime_(),
init_time_(){}

// Definitions of public member functions.
template<class UserNodeLabel, class UserEdgeLabel>
Seconds
GEDMethod<UserNodeLabel, UserEdgeLabel>::
get_runtime() const {
	return runtime_;
}

template<class UserNodeLabel, class UserEdgeLabel>
Seconds
GEDMethod<UserNodeLabel, UserEdgeLabel>::
get_init_time() const {
	return init_time_;
}

template<class UserNodeLabel, class UserEdgeLabel>
const NodeMap &
GEDMethod<UserNodeLabel, UserEdgeLabel>::
get_matching() const {
	return matching_;
}

template<class UserNodeLabel, class UserEdgeLabel>
double
GEDMethod<UserNodeLabel, UserEdgeLabel>::
get_lower_bound() const {
	return lower_bound_;
}

template<class UserNodeLabel, class UserEdgeLabel>
void
GEDMethod<UserNodeLabel, UserEdgeLabel>::
init() {
	auto start = std::chrono::high_resolution_clock::now();
	ged_init_();
	auto end = std::chrono::high_resolution_clock::now();
	init_time_ = end - start;
	initialized_ = true;
}

template<class UserNodeLabel, class UserEdgeLabel>
double
GEDMethod<UserNodeLabel, UserEdgeLabel>::
get_upper_bound() const {
	return upper_bound_;
}

template<class UserNodeLabel, class UserEdgeLabel>
void
GEDMethod<UserNodeLabel, UserEdgeLabel>::
set_options(const std::string & options) {
	read_options_from_string_(options);
	ged_set_default_options_();
	for (auto option_arg : options_) {
		if (not ged_parse_option_(option_arg.first, option_arg.second)) {
			throw Error("Invalid option \"" + option_arg.first + "\". Usage: options = \"" + ged_valid_options_string_() + "\".");
		}
	}
	initialized_ = false;
}

template<class UserNodeLabel, class UserEdgeLabel>
void
GEDMethod<UserNodeLabel, UserEdgeLabel>::
run(GEDGraph::GraphID g_id, GEDGraph::GraphID h_id) {
	Result result;
	auto start = std::chrono::high_resolution_clock::now();
	run_as_util(ged_data_.graph(g_id), ged_data_.graph(h_id), result);
	auto end = std::chrono::high_resolution_clock::now();
	lower_bound_ = result.lower_bound();
	upper_bound_ = result.upper_bound();
	if (result.num_node_maps() > 0) {
		matching_ = result.node_map(0);
	}
	runtime_ = end - start;
}

template<class UserNodeLabel, class UserEdgeLabel>
void
GEDMethod<UserNodeLabel, UserEdgeLabel>::
run_as_util(const GEDGraph & g, const GEDGraph & h, Result & result) {

	// Compute optimal solution and return if at least one of the two graphs is empty.
	if ((g.num_nodes() == 0) or (h.num_nodes() == 0)) {
		std::size_t index_node_map{result.add_node_map()};
		for (auto node = g.nodes().first; node != g.nodes().second; node++) {
			result.node_map(index_node_map).add_assignment(*node, GEDGraph::dummy_node());
		}
		for (auto node = h.nodes().first; node != h.nodes().second; node++) {
			result.node_map(index_node_map).add_assignment(GEDGraph::dummy_node(), *node);
		}
		ged_data_.compute_induced_cost(g, h, result.node_map(index_node_map));
		result.set_lower_bound(result.upper_bound());
		return;
	}

	// Run the method.
	ged_run_(g, h, result);
}

// === Definitions of private helper member functions. ===
template<class UserNodeLabel, class UserEdgeLabel>
void
GEDMethod<UserNodeLabel, UserEdgeLabel>::
read_options_from_string_(const std::string & options) {
	if (options == "") return;
	options_.clear();
	std::vector<std::string> words;
	tokenize_(options, words);
	std::string option_name;
	bool expect_option_name{true};
	for (auto word : words) {
		if (expect_option_name) {
			if (is_option_name_(word)) {
				option_name = word;
				if (options_.find(option_name) != options_.end()) {
					throw Error("Multiple specification of option \"" + option_name + "\".");
				}
				options_[option_name] = "";
			}
			else {
				throw Error("Invalid options \"" + options + "\". Usage: options = \"[--<option> <arg>] [...]\"");
			}
		}
		else {
			if (is_option_name_(word)) {
				throw Error("Invalid options \"" + options + "\". Usage: options = \"[--<option> <arg>] [...]\"");
			}
			else {
				options_[option_name] = word;
			}
		}
		expect_option_name = not expect_option_name;
	}
}

template<class UserNodeLabel, class UserEdgeLabel>
void
GEDMethod<UserNodeLabel, UserEdgeLabel>::
tokenize_(const std::string & options, std::vector<std::string> & words) const {
	bool outside_quotes{true};
	std::size_t word_length{0};
	std::size_t pos_word_start{0};
	for (std::size_t pos{0}; pos < options.size(); pos++) {
		if (options.at(pos) == '\'') {
			if (not outside_quotes and pos < options.size() - 1) {
				if (options.at(pos + 1) != ' ') {
					throw Error("Options string contains closing single quote which is followed by a char different from ' '.");
				}
			}
			word_length++;
			outside_quotes = not outside_quotes;
		}
		else if (outside_quotes and options.at(pos) == ' ') {
			if (word_length > 0) {
				words.push_back(options.substr(pos_word_start, word_length));
			}
			pos_word_start = pos + 1;
			word_length = 0;
		}
		else {
			word_length++;
		}
	}
	if (not outside_quotes) {
		throw Error("Options string contains unbalanced single quotes.");
	}
	if (word_length > 0) {
		words.push_back(options.substr(pos_word_start, word_length));
	}
}

template<class UserNodeLabel, class UserEdgeLabel>
bool
GEDMethod<UserNodeLabel, UserEdgeLabel>::
is_option_name_(std::string & word) const {
	if (word.at(0) == '\'') {
		word = word.substr(1, word.size() - 2);
		return false;
	}
	if (word.size() < 3) {
		return false;
	}
	if ((word.at(0) == '-') and (word.at(1) == '-') and (word.at(2) != '-')) {
		word = word.substr(2);
		return true;
	}
	return false;
}

// === Default definitions of private virtual member functions to be overridden by derived classes. ===
template<class UserNodeLabel, class UserEdgeLabel>
void
GEDMethod<UserNodeLabel, UserEdgeLabel>::
ged_init_() {}

template<class UserNodeLabel, class UserEdgeLabel>
void
GEDMethod<UserNodeLabel, UserEdgeLabel>::
ged_run_(const GEDGraph & g, const GEDGraph & h, Result & result) {}

template<class UserNodeLabel, class UserEdgeLabel>
bool
GEDMethod<UserNodeLabel, UserEdgeLabel>::
ged_parse_option_(const std::string & option, const std::string & arg) {
	return false;
}

template<class UserNodeLabel, class UserEdgeLabel>
std::string
GEDMethod<UserNodeLabel, UserEdgeLabel>::
ged_valid_options_string_() const {
	return "";
}

template<class UserNodeLabel, class UserEdgeLabel>
void
GEDMethod<UserNodeLabel, UserEdgeLabel>::
ged_set_default_options_() {}

}

#endif /* SRC_METHODS_GED_METHOD_IPP_ */