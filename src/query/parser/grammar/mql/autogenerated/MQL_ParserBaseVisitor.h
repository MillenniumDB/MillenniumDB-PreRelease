
// Generated from MQL_Parser.g4 by ANTLR 4.13.1

#pragma once


#include "antlr4-runtime.h"
#include "MQL_ParserVisitor.h"


/**
 * This class provides an empty implementation of MQL_ParserVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  MQL_ParserBaseVisitor : public MQL_ParserVisitor {
public:

  virtual std::any visitRoot(MQL_Parser::RootContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMatchQuery(MQL_Parser::MatchQueryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitProjectSimilarity(MQL_Parser::ProjectSimilarityContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMetricType(MQL_Parser::MetricTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDescribeQuery(MQL_Parser::DescribeQueryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDescribeFlag(MQL_Parser::DescribeFlagContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInsertQuery(MQL_Parser::InsertQueryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInsertLabelList(MQL_Parser::InsertLabelListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInsertPropertyList(MQL_Parser::InsertPropertyListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInsertEdgeList(MQL_Parser::InsertEdgeListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInsertLabelElement(MQL_Parser::InsertLabelElementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInsertPropertyElement(MQL_Parser::InsertPropertyElementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInsertEdgeElement(MQL_Parser::InsertEdgeElementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSetStatement(MQL_Parser::SetStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMatchStatement(MQL_Parser::MatchStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhereStatement(MQL_Parser::WhereStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGroupByStatement(MQL_Parser::GroupByStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrderByStatement(MQL_Parser::OrderByStatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturnList(MQL_Parser::ReturnListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturnAll(MQL_Parser::ReturnAllContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSetItem(MQL_Parser::SetItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturnItemVar(MQL_Parser::ReturnItemVarContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturnItemAgg(MQL_Parser::ReturnItemAggContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturnItemCount(MQL_Parser::ReturnItemCountContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAggregateFunc(MQL_Parser::AggregateFuncContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrderByItemVar(MQL_Parser::OrderByItemVarContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrderByItemAgg(MQL_Parser::OrderByItemAggContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrderByItemCount(MQL_Parser::OrderByItemCountContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGroupByItem(MQL_Parser::GroupByItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitGraphPattern(MQL_Parser::GraphPatternContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOptionalPattern(MQL_Parser::OptionalPatternContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimilaritySearch(MQL_Parser::SimilaritySearchContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTensor(MQL_Parser::TensorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBasicPattern(MQL_Parser::BasicPatternContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLinearPattern(MQL_Parser::LinearPatternContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPath(MQL_Parser::PathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPathAlternatives(MQL_Parser::PathAlternativesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPathSequence(MQL_Parser::PathSequenceContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPathAtomSimple(MQL_Parser::PathAtomSimpleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPathAtomAlternatives(MQL_Parser::PathAtomAlternativesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPathSuffix(MQL_Parser::PathSuffixContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPathType(MQL_Parser::PathTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNode(MQL_Parser::NodeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFixedNode(MQL_Parser::FixedNodeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFixedNodeInside(MQL_Parser::FixedNodeInsideContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEdge(MQL_Parser::EdgeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEdgeInside(MQL_Parser::EdgeInsideContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVarNode(MQL_Parser::VarNodeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitProperties(MQL_Parser::PropertiesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitProperty1(MQL_Parser::Property1Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitProperty2(MQL_Parser::Property2Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIdentifier(MQL_Parser::IdentifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBoolValue(MQL_Parser::BoolValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNumericValue(MQL_Parser::NumericValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDatatypeValue(MQL_Parser::DatatypeValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue(MQL_Parser::ValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConditionalOrExpr(MQL_Parser::ConditionalOrExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConditionalAndExpr(MQL_Parser::ConditionalAndExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitComparisonExprOp(MQL_Parser::ComparisonExprOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitComparisonExprIs(MQL_Parser::ComparisonExprIsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAdditiveExpr(MQL_Parser::AdditiveExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMultiplicativeExpr(MQL_Parser::MultiplicativeExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnaryExpr(MQL_Parser::UnaryExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExprVar(MQL_Parser::ExprVarContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExprValueExpr(MQL_Parser::ExprValueExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExprParenthesis(MQL_Parser::ExprParenthesisContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValueExpr(MQL_Parser::ValueExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExprTypename(MQL_Parser::ExprTypenameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitKeyword(MQL_Parser::KeywordContext *ctx) override {
    return visitChildren(ctx);
  }


};

