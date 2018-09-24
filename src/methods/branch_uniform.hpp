/*!
 *  @file  branch_uniform.hpp
 *  @brief ged::BranchUniform class declaration.
 */

#ifndef SRC_METHODS_BRANCH_UNIFORM_HPP_
#define SRC_METHODS_BRANCH_UNIFORM_HPP_

namespace ged {

/*!
 * @brief Computes lower and upper bounds for uniform edit costs.
 * @details Implements the method %BranchUniform suggested in:
 * - W. Zheng, L. Zou, X. Lian, D. Wang, and D. Zhao:
 *   &ldquo;Efficient graph similarity search over large graph databases&rdquo;,
 *   https:://doi.org/10.1109/TKDE.2014.2349924
 *
 * Supports the following option in addition to the ones supported by ged::LSAPEBasedMethod:
 * | <tt>\--@<option@> @<arg@></tt> | modified parameter | default  | more information |
 * | ------------------------------ | ------------------ | -------- | ---------------- |
 * | <tt>\--sort-method STD\|COUNTING</tt> | the employed sorting algorithm | @p COUNTING | @ref ged::util::counting_sort() <br> use counting sort if the number of different edge labels is constant |
 * | <tt>\--wildcards YES\|NO</tt> | decide if wildcards should be used | @p NO | https:://doi.org/10.1109/TKDE.2014.2349924 |
 */
template<class UserNodeLabel, class UserEdgeLabel>
class BranchUniform : public LSAPEBasedMethod<UserNodeLabel, UserEdgeLabel> {

public:
	virtual ~BranchUniform();

	BranchUniform(const GEDData<UserNodeLabel, UserEdgeLabel> & ged_data);

private:

	enum SortMethod_ {STD, COUNTING};

	class SortedUserEdgeLabels_ {
	public:
		SortedUserEdgeLabels_(const GEDGraph & g, SortMethod_ sort_method);

		SortedUserEdgeLabels_();

		void operator=(const SortedUserEdgeLabels_ & sorted_edge_labels);

		const std::vector<LabelID> & get_incident_labels(GEDGraph::NodeID) const;

	private:
		std::map<GEDGraph::NodeID, std::vector<LabelID>> sorted_edge_labels_;
	};

	SortMethod_ sort_method_;

	bool wildcard_option_;

	std::map<GEDGraph::GraphID, SortedUserEdgeLabels_> sorted_edge_labels_;

	// Member functions inherited from LSAPEBasedMethod.

	virtual bool lsape_parse_option_(const std::string & option, const std::string & arg) final;

	virtual std::string lsape_valid_options_string_() const final;

	virtual void lsape_set_default_options_() final;

	virtual void lsape_populate_instance_(const GEDGraph & g, const GEDGraph & h, DMatrix & master_problem) final;

	virtual void lsape_init_graph_(const GEDGraph & graph) final;

	// Private helper member functions.

	double compute_substitution_cost_(const GEDGraph & g, const GEDGraph & h, GEDGraph::NodeID i, GEDGraph::NodeID k,
			const SortedUserEdgeLabels_ & sorted_edge_labels_g, const SortedUserEdgeLabels_ & sorted_edge_labels_h) const;

	double compute_wildcard_substitution_cost_(const GEDGraph & g, const GEDGraph & h, GEDGraph::NodeID i, GEDGraph::NodeID k) const;

	double compute_deletion_cost_(const GEDGraph & g, GEDGraph::NodeID i) const;

	double compute_insertion_cost_(const GEDGraph & h, GEDGraph::NodeID k) const;

	double compute_wildcard_insertion_cost_(const GEDGraph & h, GEDGraph::NodeID k) const;
};

}

#endif /* SRC_METHODS_BRANCH_UNIFORM_HPP_ */