#include <iostream>
#include "SQLParser.h"
#include "util/sqlhelper.h"

int main() {
	const std::string query = "SELECT * from A where A.x > 0;";
    hsql::SQLParserResult result;
    hsql::SQLParser::parse(query, &result);

    if (result.isValid() && result.size() > 0) {
        const hsql::SQLStatement* statement = result.getStatement(0);

        if (statement->isType(hsql::StatementType::kStmtSelect)) {
            const hsql::SelectStatement* select = (const hsql::SelectStatement*) statement;
            /* ... */
			hsql::printStatementInfo(select);
        }
    }
	return 0;
}
