#pragma once

#include "antlr4-runtime.h"

#include "misc/logger.h"
#include "query/parser/grammar/sparql/update_autogenerated/SparqlUpdateLexer.h"
#include "query/parser/grammar/sparql/update_autogenerated/SparqlUpdateParser.h"
#include "query/parser/grammar/sparql/update_visitor.h"
#include "query/parser/op/sparql/update/op_update.h"

namespace SPARQL {

class UpdateParser {
public:
    static std::unique_ptr<OpUpdate> get_query_plan(
        const std::string& query,
        antlr4::ANTLRErrorListener* error_listener = nullptr)
    {
        antlr4::ANTLRInputStream  input(query);
        SparqlUpdateLexer         lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        SparqlUpdateParser        parser(&tokens);

        parser.getInterpreter<antlr4::atn::ParserATNSimulator>()->setPredictionMode(
            antlr4::atn::PredictionMode::SLL
        );

        if (error_listener != nullptr) {
            parser.removeErrorListeners();
            parser.addErrorListener(error_listener);
        }
        SparqlUpdateParser::QueryContext* tree = parser.query();

        UpdateVisitor visitor;
        visitor.visitQuery(tree);

        logger(Category::LogicalPlan) << "Update logical plan:\n" << *visitor.op_update;

        return std::move(visitor.op_update);
    }
};
} // namespace SPARQL