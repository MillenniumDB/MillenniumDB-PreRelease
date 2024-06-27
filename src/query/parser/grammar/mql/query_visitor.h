#pragma once

#include <memory>
#include <vector>

#include "antlr4-runtime.h"
#include "graph_models/object_id.h"
#include "query/query_context.h"
#include "query/var_id.h"
#include "query/parser/grammar/mql/autogenerated/MQL_ParserBaseVisitor.h"
#include "query/parser/op/mql/ops.h"
#include "query/parser/paths/regular_path_expr.h"

class Expr;

namespace LSH {
enum class MetricType;
}

namespace MQL {

class OpBasicGraphPattern;

class QueryVisitor : public MQL_ParserBaseVisitor {
private:
    struct ReturnInfo {
        std::vector<std::pair<VarId, std::unique_ptr<Expr>>> items;
        bool distinct;
        uint64_t limit;
    };

    struct OrderByInfo {
        std::vector<std::pair<VarId, std::unique_ptr<Expr>>> items;

        // must have the same size as items
        std::vector<bool> ascending_order;
    };

    struct SimilaritySortInfo {
        VarId              object_var { 0 };
        VarId              similarity_var { 0 };
        std::string        tensor_store_name;
        std::vector<float> query_tensor;
        ObjectId           query_object;
        LSH::MetricType    metric_type;
    };

    ReturnInfo return_info;

    OrderByInfo order_by_info;

    SimilaritySortInfo similarity_sort_info;

    std::vector<std::pair<VarId, ObjectId>> set_items;

    // Properties used declared inside MATCH
    std::set<OpProperty> match_var_properties;

    std::vector<VarId> group_by_vars;

    std::unique_ptr<Expr> current_expr;

    std::unique_ptr<OpBasicGraphPattern> current_basic_graph_pattern;

    std::set<VarId> possible_disjoint_vars;

    std::unique_ptr<RegularPathExpr> current_path;

    std::vector<OpLabel> insert_labels;

    std::vector<OpProperty> insert_properties;

    std::vector<OpEdge> insert_edges;

    // to detect possible disjoint vars / terms
    // initialized false to avoid calling
    // current_basic_graph_pattern->add_disjoint_term (segfault)
    // when seeing a DESCRIBE query
    bool first_element_disjoint = false;

    bool current_path_inverse;

    Id last_node = ObjectId::get_null();

    Id saved_node = ObjectId::get_null();

    Id saved_edge = ObjectId::get_null();

    Id saved_type = ObjectId::get_null();

    std::vector<float> current_tensor;

    Id saved_property_obj = ObjectId::get_null();

public:
    std::unique_ptr<Op> current_op;

    // Properties used outside MATCH (WHERE/GROUP BY/ORDER BY/RETURN)
    std::set<OpProperty> used_var_properties;

    virtual std::any visitDescribeQuery(MQL_Parser::DescribeQueryContext*) override;
    virtual std::any visitInsertQuery(MQL_Parser::InsertQueryContext* ctx) override;
    virtual std::any visitMatchQuery(MQL_Parser::MatchQueryContext* ctx) override;
    virtual std::any visitMatchStatement(MQL_Parser::MatchStatementContext* ctx) override;
    virtual std::any visitSetItem(MQL_Parser::SetItemContext* ctx) override;

    virtual std::any visitInsertLabelElement(MQL_Parser::InsertLabelElementContext* ctx) override;
    virtual std::any visitInsertPropertyElement(MQL_Parser::InsertPropertyElementContext* ctx) override;
    virtual std::any visitInsertEdgeElement(MQL_Parser::InsertEdgeElementContext* ctx) override;

    virtual std::any visitReturnList(MQL_Parser::ReturnListContext* ctx) override;
    virtual std::any visitReturnItemVar(MQL_Parser::ReturnItemVarContext* ctx) override;
    virtual std::any visitReturnItemAgg(MQL_Parser::ReturnItemAggContext* ctx) override;
    virtual std::any visitReturnItemCount(MQL_Parser::ReturnItemCountContext* ctx) override;
    virtual std::any visitReturnAll(MQL_Parser::ReturnAllContext* ctx) override;

    virtual std::any visitOrderByStatement(MQL_Parser::OrderByStatementContext* ctx) override;
    virtual std::any visitOrderByItemVar(MQL_Parser::OrderByItemVarContext*ctx) override;
    virtual std::any visitOrderByItemAgg(MQL_Parser::OrderByItemAggContext* ctx) override;
    virtual std::any visitOrderByItemCount(MQL_Parser::OrderByItemCountContext* ctx) override;

    virtual std::any visitGroupByStatement(MQL_Parser::GroupByStatementContext* ctx) override;
    virtual std::any visitGroupByItem(MQL_Parser::GroupByItemContext* ctx) override;

    virtual std::any visitGraphPattern(MQL_Parser::GraphPatternContext* ctx) override;
    virtual std::any visitBasicPattern(MQL_Parser::BasicPatternContext* ctx) override ;
    virtual std::any visitLinearPattern(MQL_Parser::LinearPatternContext* ctx) override;
    virtual std::any visitFixedNodeInside(MQL_Parser::FixedNodeInsideContext* ctx) override;
    virtual std::any visitVarNode(MQL_Parser::VarNodeContext* ctx) override;
    virtual std::any visitEdge(MQL_Parser::EdgeContext* ctx) override;
    virtual std::any visitEdgeInside(MQL_Parser::EdgeInsideContext* ctx) override;
    virtual std::any visitProperty1(MQL_Parser::Property1Context* ctx) override;
    virtual std::any visitProperty2(MQL_Parser::Property2Context* ctx) override;

    virtual std::any visitPath(MQL_Parser::PathContext* ctx) override;
    virtual std::any visitPathAlternatives(MQL_Parser::PathAlternativesContext* ctx) override;
    virtual std::any visitPathSequence(MQL_Parser::PathSequenceContext* ctx) override;
    virtual std::any visitPathAtomSimple(MQL_Parser::PathAtomSimpleContext* ctx) override;
    virtual std::any visitPathAtomAlternatives(MQL_Parser::PathAtomAlternativesContext* ctx) override;

    virtual std::any visitWhereStatement(MQL_Parser::WhereStatementContext* ctx) override;

    virtual std::any visitExprVar(MQL_Parser::ExprVarContext* ctx) override;
    virtual std::any visitExprValueExpr(MQL_Parser::ExprValueExprContext* ctx) override;

    virtual std::any visitConditionalOrExpr(MQL_Parser::ConditionalOrExprContext* ctx) override;
    virtual std::any visitConditionalAndExpr(MQL_Parser::ConditionalAndExprContext* ctx) override;
    virtual std::any visitComparisonExprOp(MQL_Parser::ComparisonExprOpContext* ctx) override;
    virtual std::any visitComparisonExprIs(MQL_Parser::ComparisonExprIsContext* ctx) override;
    virtual std::any visitAdditiveExpr(MQL_Parser::AdditiveExprContext* ctx) override;
    virtual std::any visitMultiplicativeExpr(MQL_Parser::MultiplicativeExprContext* ctx) override;
    virtual std::any visitUnaryExpr(MQL_Parser::UnaryExprContext* ctx) override;
  
    virtual std::any visitSimilaritySearch(MQL_Parser::SimilaritySearchContext* ctx) override;
    virtual std::any visitProjectSimilarity(MQL_Parser::ProjectSimilarityContext* ctx) override;
    virtual std::any visitTensor(MQL_Parser::TensorContext* ctx) override;
};
} // namespace MQL
