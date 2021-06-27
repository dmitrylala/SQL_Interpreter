СУБД:
Реализована In-Memory СУБД (модуль dbms), поддерживающая следующие операции: добавление записи, удаление записи, получение доступа к записи (и как следствие редактирование), открытие/создание/удаление таблицы, также можно итерироваться по записям. Поля данных возможны двух типов: Long и Text.
Архитектура: класс для таблицы Table, TableStruct - класс для структур таблиц; структура записывается в первую строчку файла в виде: type_of_field, name, max_len (максимальная длина задается только для текстовых полей). Записи записываются в файл в формате csv.

Сервер и клиент на сокетах:
Реализован интерефейс для работы с сокетами в объектно-ориентированном стиле (модуль sock_wrap). Программы клиента и сервера приведены в client.cpp и server.cpp соответственно. Клиент считывает строку символов до '\n', после чего передает запрос серверу. Сервер обрабатывает запрос, и отсылает клиенту таблицу-ответ или сообщение об ошибке. Клиент может отправлять несколько запросов по очереди, прекращение работы клиента по EOF. Прекращение работы сервера по терминальным сигналам SIGTERM или SIGINT.
Архитектура сокетов: базовые классы - SocketAddress и BaseSocket, от них наследуются UnSocketAddress (для struct sockaddr_un), ServerSocket, ClientSocket и тд.

Интерпретатор:
Реализован интерпретатор подмножества языка SQL (модуль Parser). Лексический анализатор (модуль Scanner) считывает символы со стандартного потока ввода и формирует лексемы, парсер проводит синтаксический анализ получаемой последовательности лексем методом рекурсивного спуска по приведенной ниже формальной грамматике (это измененная таким образом исходная грамматика, чтобы к ней был применим РС-метод), формирует запрос во внутреннем представлении (в виде ПОЛИЗа), затем выполняет его.

Запуск и тесты:
В makefile есть цели для сборки как в основном режиме (make all - компиляция клиента и сервера), так и в тестовом (make test или make test_cov для последующей генерации отчета о покрытии кода при помощи команды make test_code_coverage), все тесты приведены в DerivedTests.cpp (тесты СУБД, сканнера и парсера).

Обработка ошибок: 
СУБД обрабатывает запросы к несуществующим полям, не вносит в таблицу текстовое значение, если его длина больше максимальной длины соответствующего текстового поля, также фиксируются различные другие семантические ошибки - генерируя соответствующие исключительные ситуации. Сканнер и парсер также обрабатывают всевозможные лексические, синтаксические и семантические ошибки. Иерархия служебных классов для исключений приведена в модуле Exceptions.

Новая грамматика:
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
