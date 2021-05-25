Обозначения: в двойных кавычках указаны терминалы, заглавными буквами - нетерминалы; запись по типу ("+" | "-") 
обозначает выбор одной из альтернатив в скобках; { } и [ ] - итерации, в угловых скобках < > указаны действия; eps - пустая цепочка

Грамматика G', полученная из исходной G:
1. START -> "INSERT" INSERT | "CREATE" CREATE | "DROP" DROP | "UPDATE" UPDATE | "SELECT" SELECT | "DELETE" DELETE

2. INSERT -> "INTO" "table_id" "(" FIELD_VAL { "," FIELD_VAL } ")"
3. FIELD_VAL -> "str" | "num"

4. CREATE -> "TABLE" "table_id" "(" "field_id" FIELD_TYPE { "," "field_id" FIELD_TYPE } ")"
5. FIELD_TYPE -> "TEXT" "(" "num" ")" | "LONG"

6. DROP -> "TABLE" "table_id"

7. UPDATE -> "table_id" "SET" "field_id" "=" EXPR "WHERE" WHERE

8. SELECT -> FIELDS "FROM" "table_id" "WHERE" WHERE
9. FIELDS -> "field_id" { "," "field_id" } | "*"

10. DELETE -> "FROM" "table_id" "WHERE" WHERE

11. WHERE -> EXPR_BOOL [<if is_expr==true>[ "NOT" ] OPTION ] | "ALL"
12. OPTION -> "IN" "(" CONST_LIST ")" | <if is_text_field==true> "LIKE" "str"
13. CONST_LIST -> "str" { ","  "str" } | "num" { "," "num" }

14. EXPR_BOOL -> BOOL_TERM { "OR" <is_expr=false> BOOL_TERM }
15. BOOL_TERM -> BOOL_MULT { "AND" <is_expr=false> BOOL_MULT }
16. BOOL_MULT -> "NOT" <is_expr=false> BOOL_MULT | EXPR [ ("=" | "!=" | "<" | ">" | "<=" | ">=") <is_expr=false> <if is_inside==false throw err> EXPR ] | "(" <is_inside=true> EXPR_BOOL ")" <is_inside=false>

17. EXPR -> EXPR_LONG | "field_text_id" <is_text_field=true> | "str"
18. EXPR_LONG -> LONG_TERM { ("+"|"-") LONG_TERM }
19. LONG_TERM -> LONG_MULT { ("*"|"/"|"%") LONG_MULT }
20. LONG_MULT -> "field_long_id" | "num" | "(" EXPR_LONG ")"

Данная грамматика G', если не учитывать действия, порождает больший язык, чем L(G), однако засчет действий цепочки из L(G'), не входящие в L(G), считаются синтаксически недопустимыми.
При этом к данной грамматике применим метод рекурсивного спуска:

Правила 6, 7, 8 и 10 альтернатив не имеют, значит, к ним применим РС-метод.
Альтернативы в правилах 1, 3, 5, 12 и 20 начинаются с разных терминалов, значит, к ним применим РС-метод.

Правила вида A -> A { "a" A } можно привести к паре правил вида
	A -> "a" B
	B -> "a" B | eps
К которым РС-метод уже применим. Поэтому, к правилам 14, 15, 18 и 19 применим РС-метод; также, в правиле 2 можно провести аналогичную замену:
	INSERT -> "INTO" "table_id" "(" B ")"
	B -> FIELD_VAL { "," FIELD_VAL }
При этом РС-метод будет применим, так как first(B) не пересекается с follow(B); таким же образом, проведя аналогичную замену в правиле 4 (добавив новый нетерминал B), будет получена пара правил, к которым тоже применим РС-метод, так как
first(B) не пересекается с follow(B).
Альтернативы в правилах 9 и 13 начинаются с разных терминалов, и итерации могут быть заменены с помощью дополнительного нетерминала => к правилам 9 и 13 применим РС-метод. В правиле 11 не пересекаются first и follow (терминал "ALL" потом не встречается). У 16 и 17 правила 2 альтернативы начинаются с разных терминалов, при этом first и follow снова не пересекаются.

Парсер, реализующий метод рекурсивного спуска по построенной грамматике, а также вычисляющий полиз, находится в Parser.cpp. В Scanner.cpp находится лексический анализатор.

В makefile есть цели для сборки как в основном режиме (make all - компиляция клиента и сервера), так и в тестовом (make test или make test_cov для последующей генерации отчета о покрытии), все тесты приведены в DerivedTests.cpp.

Клиент считывает строку символов до '\n', после чего передает запрос серверу. Сервер обрабатывает запрос, и отсылает клиенту таблицу-ответ или сообщение об ошибке. Клиент может отправлять несколько запросов по очереди, прекращение работы клиента по EOF. Прекращение работы сервера по терминальным сигналам SIGTERM или SIGINT.
