```bash
root@99a7a91b30a1:/app# ./bin/HackyDb

               ██╗  ██╗ █████╗  ██████╗██╗   ██╗██╗   ██╗
               ██║  ██║██╔══██╗██╔════╝██║  ██║ ╚██╗ ██╔╝
               ███████║███████║██║     ████║     ╚████╔╝
               ██╔══██║██╔══██║██║     ██╔═ ██║   ╚██╔╝
               ██║  ██║██║  ██║╚██████╗██║   ██║   ██║
               ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝   ╚═╝   ╚═╝

═══════════════════════════════════════════════════════════════════════════════
               Welcome to HackyDB - Your Very Own Database System!
═══════════════════════════════════════════════════════════════════════════════

         HackyDB is a powerful, complete database system built in C++.
    With support for all core database operations including system calls,
     indexing, file handling, and record management, you're ready to go!
    HackyDB integrates OS-level data management to bring you a truly robust
    solution for managing data efficiently. Start exploring and building!

═══════════════════════════════════════════════════════════════════════════════
         Ready to manage data with full control? Let's dive into HackyDB!
═══════════════════════════════════════════════════════════════════════════════
🔹 Project: HackyDB
🔹 Course: CS315 - DATABASE SYSTEMS (6th Semester)
🔹 Institute: Indian Institute of Technology Kanpur
🔹 Instructor: Prof. Arnab Bhattacharya

🧩 Modules: Interpreter | API | Record Manager | Index Manager | Catalog Manager | Buffer Manager (LRU Cache with 4KB Blocks)

👨‍💻 Crafted With Passion By Team HACKY:
   ⚡ Harshit            ⚡ Aditya Gupta
   ⚡ Chayan Kumawat     ⚡ Kushagra Singh
   ⚡ Yash Pratap Singh

💡 Tips:
   • End SQL statements with ';'
   • Type 'help' to see available commands
   • Type 'exit' or 'quit' to close HackyDB
═══════════════════════════════════════════════════════════════════════════════

[HackyDB] Ready to execute your SQL commands. Type 'help' for assistance.

Supported SQL Queries:
-----------------------
1. SELECT * FROM table_name [WHERE column = value [AND ...]]
2. INSERT INTO table_name VALUES (value1, value2, ...)
3. DELETE FROM table_name [WHERE column = value [AND ...]]
4. CREATE DATABASE database_name
5. DROP DATABASE database_name
6. USE database_name
7. CREATE TABLE table_name (column_name TYPE, ..., PRIMARY KEY(column_name))
8. DROP TABLE table_name
9. CREATE INDEX index_name ON table_name(column_name)
10. DROP INDEX index_name
11. EXEC file_name

Note:
- Types: INT, FLOAT, CHAR(n)
- CHAR values must be enclosed in single ('') or double quotes ("")
- WHERE conditions support: =, <, >, <=, >=, <>

HackyDB> SHOW DATABASES;
SQL TYPE: #SHOW DATABASES#
SQL STATEMENT: SHOW DATABASES
DATABASE LIST:
	DB1


HackyDB> CREATE DATABASE DB2;
SQL TYPE: #CREATE DATABASE#
SQL STATEMENT: CREATE DATABASE DB2
DB NAME: DB2
Creating database: DB2
Database folder created!
Catalog written!


HackyDB> USE DB2;
SQL TYPE: #USE#
SQL STATEMENT: USE DB2
DB NAME: DB2


HackyDB> create table students (
    student_id int,
    name char(20),
    department char(10),
    cgpa float,
    primary key (student_id)
);
SQL TYPE: #CREATE TABLE#
SQL STATEMENT: create table students ( student_id int , name char ( 20 )  , department char ( 10 )  , cgpa float , primary key ( student_id )  )
TABLE NAME: students
COLUMN: student_id
TYPE: int
COLUMN: name
COLUMN: department
COLUMN: cgpa
TYPE: float
PRIMARY KEY: student_id
Creating table: students
Table file created!
Catalog written!


HackyDB> insert into students values (101, 'Alice', 'CSE', 9.1);
SQL TYPE: #INSERT#
SQL STATEMENT: insert into students values ( 101 , 'Alice' , 'CSE' , 9.1 )
TABLE NAME: students
0 : 101
2 : Alice
2 : CSE
1 : 9.1


HackyDB> insert into students values (102, 'Bob', 'EE', 8.5);
SQL TYPE: #INSERT#
SQL STATEMENT: insert into students values ( 102 , 'Bob' , 'EE' , 8.5 )
TABLE NAME: students
0 : 102
2 : Bob
2 : EE
1 : 8.5


HackyDB> insert into students values (103, 'Charlie', 'ME', 7.2);
SQL TYPE: #INSERT#
SQL STATEMENT: insert into students values ( 103 , 'Charlie' , 'ME' , 7.2 )
TABLE NAME: students
0 : 103
2 : Charlie
2 : ME
1 : 7.2


HackyDB> insert into students values (104, 'David', 'CSE', 8.8);
SQL TYPE: #INSERT#
SQL STATEMENT: insert into students values ( 104 , 'David' , 'CSE' , 8.8 )
TABLE NAME: students
0 : 104
2 : David
2 : CSE
1 : 8.8


HackyDB> insert into students values (105, 'Eve', 'EE', 9.0);
SQL TYPE: #INSERT#
SQL STATEMENT: insert into students values ( 105 , 'Eve' , 'EE' , 9.0 )
TABLE NAME: students
0 : 105
2 : Eve
2 : EE
1 : 9.0


HackyDB> select * from students where cgpa > 8.5;
SQL TYPE: #SELECT#
SQL STATEMENT: select * from students where cgpa > 8.5
TABLE NAME: students
cgpa 3 8.5
student_idname     departmentcgpa
101      Alice    CSE      9.1
104      David    CSE      8.8
105      Eve      EE       9


HackyDB> update students set cgpa = 9.5 where student_id = 105;
SQL TYPE: #UPDATE#
SQL STATEMENT: update students set cgpa = 9.5 where student_id = 105
TABLE NAME: students
cgpa 9.5
student_id 0 105


HackyDB> delete from students where student_id = 103;
SQL TYPE: #DELETE#
SQL STATEMENT: delete from students where student_id = 103
TABLE NAME: students
student_id 0 103


HackyDB> create index idx_stud_id on students(student_id);
SQL TYPE: #CREATE INDEX#
SQL STATEMENT: create index idx_stud_id on students ( student_id )
INDEX NAME: idx_stud_id
TABLE NAME: students
COLUMN NAME: student_id
*****************************************************
KeyCount: 4, NodeCount: 1, Level: 1, Root: 0
----------------------
BlockNum: 0 Count: 4, Parent: -1  IsLeaf:1
Keys: { 101      102      104      105       }
Vals: { 0000000 0000001 0000003 0000002  }
NextLeaf:    -1


HackyDB> insert into students values (106, 'Frank', 'ME', 7.8);
SQL TYPE: #INSERT#
SQL STATEMENT: insert into students values ( 106 , 'Frank' , 'ME' , 7.8 )
TABLE NAME: students
0 : 106
2 : Frank
2 : ME
1 : 7.8


HackyDB> insert into students values (107, 'Grace', 'CSE', 9.3);
insert into students values (108, 'Heidi', 'EE', 6.9);
SQL TYPE: #INSERT#
SQL STATEMENT: insert into students values ( 107 , 'Grace' , 'CSE' , 9.3 )
TABLE NAME: students
0 : 107
2 : Grace
2 : CSE
1 : 9.3


HackyDB> insert into students values (108, 'Heidi', 'EE', 6.9);
SQL TYPE: #INSERT#
SQL STATEMENT: insert into students values ( 108 , 'Heidi' , 'EE' , 6.9 )
TABLE NAME: students
0 : 108
2 : Heidi
2 : EE
1 : 6.9


HackyDB> insert into students values (109, 'Ivan', 'ME', 8.2);
SQL TYPE: #INSERT#
SQL STATEMENT: insert into students values ( 109 , 'Ivan' , 'ME' , 8.2 )
TABLE NAME: students
0 : 109
2 : Ivan
2 : ME
1 : 8.2


HackyDB> insert into students values (109, 'Ivan', 'ME', 8.2);
SQL TYPE: #INSERT#
SQL STATEMENT: insert into students values ( 109 , 'Ivan' , 'ME' , 8.2 )
TABLE NAME: students
0 : 109
2 : Ivan
2 : ME
1 : 8.2
Primary key conflicts!


HackyDB> insert into students values (110, 'Judy', 'CSE', 9.7);
SQL TYPE: #INSERT#
SQL STATEMENT: insert into students values ( 110 , 'Judy' , 'CSE' , 9.7 )
TABLE NAME: students
0 : 110
2 : Judy
2 : CSE
1 : 9.7


HackyDB> select * from students where department = 'CSE';
SQL TYPE: #SELECT#
SQL STATEMENT: select * from students where department = 'CSE'
TABLE NAME: students
department 0 CSE
student_idname     departmentcgpa
101      Alice    CSE      9.1
104      David    CSE      8.8
107      Grace    CSE      9.3
110      Judy     CSE      9.7
*****************************************************
KeyCount: 9, NodeCount: 1, Level: 1, Root: 0
----------------------
BlockNum: 0 Count: 9, Parent: -1  IsLeaf:1
Keys: { 101      102      104      105      106      107      108      109      110       }
Vals: { 0000000 0000001 0000003 0000002 0000004 0000005 0000006 0000007 0000008  }
NextLeaf:    -1


HackyDB> select * from students where cgpa > 8.0 and cgpa < 9.5;
SQL TYPE: #SELECT#
SQL STATEMENT: select * from students where cgpa > 8.0 and cgpa < 9.5
TABLE NAME: students
cgpa 3 8.0
cgpa 2 9.5
student_idname     departmentcgpa
101      Alice    CSE      9.1
102      Bob      EE       8.5
104      David    CSE      8.8
107      Grace    CSE      9.3
109      Ivan     ME       8.2
*****************************************************
KeyCount: 9, NodeCount: 1, Level: 1, Root: 0
----------------------
BlockNum: 0 Count: 9, Parent: -1  IsLeaf:1
Keys: { 101      102      104      105      106      107      108      109      110       }
Vals: { 0000000 0000001 0000003 0000002 0000004 0000005 0000006 0000007 0000008  }
NextLeaf:    -1


HackyDB> update students set department = 'DS' where student_id = 108;
SQL TYPE: #UPDATE#
SQL STATEMENT: update students set department = 'DS' where student_id = 108
TABLE NAME: students
department DS
student_id 0 108


HackyDB> update students set cgpa = 8.7 where student_id = 102;
SQL TYPE: #UPDATE#
SQL STATEMENT: update students set cgpa = 8.7 where student_id = 102
TABLE NAME: students
cgpa 8.7
student_id 0 102


HackyDB> update students set cgpa = 9.2 where student_id = 105;
SQL TYPE: #UPDATE#
SQL STATEMENT: update students set cgpa = 9.2 where student_id = 105
TABLE NAME: students
cgpa 9.2
student_id 0 105


HackyDB> delete from students where student_id = 108;
SQL TYPE: #DELETE#
SQL STATEMENT: delete from students where student_id = 108
TABLE NAME: students
student_id 0 108


HackyDB> delete from students where student_id = 106;
SQL TYPE: #DELETE#
SQL STATEMENT: delete from students where student_id = 106
TABLE NAME: students
student_id 0 106


HackyDB> select * from students where cgpa >= 9.0;
SQL TYPE: #SELECT#
SQL STATEMENT: select * from students where cgpa >= 9.0
TABLE NAME: students
cgpa 5 9.0
student_idname     departmentcgpa
101      Alice    CSE      9.1
105      Eve      EE       9.2
107      Grace    CSE      9.3
110      Judy     CSE      9.7
*****************************************************
KeyCount: 7, NodeCount: 1, Level: 1, Root: 0
----------------------
BlockNum: 0 Count: 7, Parent: -1  IsLeaf:1
Keys: { 101      102      104      105      107      109      110       }
Vals: { 0000000 0000001 0000003 0000002 0000005 0000007 0000008  }
NextLeaf:    -1


HackyDB> insert into students values (106, 'Frank', 'ME', 8.0);
SQL TYPE: #INSERT#
SQL STATEMENT: insert into students values ( 106 , 'Frank' , 'ME' , 8.0 )
TABLE NAME: students
0 : 106
2 : Frank
2 : ME
1 : 8.0


HackyDB> select * from students where student_id = 107;
SQL TYPE: #SELECT#
SQL STATEMENT: select * from students where student_id = 107
TABLE NAME: students
student_id 0 107
student_idname     departmentcgpa
107      Grace    CSE      9.3
*****************************************************
KeyCount: 8, NodeCount: 1, Level: 1, Root: 0
----------------------
BlockNum: 0 Count: 8, Parent: -1  IsLeaf:1
Keys: { 101      102      104      105      106      107      109      110       }
Vals: { 0000000 0000001 0000003 0000002 0000007 0000005 0000007 0000008  }
NextLeaf:    -1



HackyDB> Quit;
SQL TYPE: #QUIT#
SQL STATEMENT: Quit
Quiting...
root@99a7a91b30a1:/app# ./bin/HackyDb

               ██╗  ██╗ █████╗  ██████╗██╗   ██╗██╗   ██╗
               ██║  ██║██╔══██╗██╔════╝██║  ██║ ╚██╗ ██╔╝
               ███████║███████║██║     ████║     ╚████╔╝
               ██╔══██║██╔══██║██║     ██╔═ ██║   ╚██╔╝
               ██║  ██║██║  ██║╚██████╗██║   ██║   ██║
               ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝   ╚═╝   ╚═╝

═══════════════════════════════════════════════════════════════════════════════
               Welcome to HackyDB - Your Very Own Database System!
═══════════════════════════════════════════════════════════════════════════════

         HackyDB is a powerful, complete database system built in C++.
    With support for all core database operations including system calls,
     indexing, file handling, and record management, you're ready to go!
    HackyDB integrates OS-level data management to bring you a truly robust
    solution for managing data efficiently. Start exploring and building!

═══════════════════════════════════════════════════════════════════════════════
         Ready to manage data with full control? Let's dive into HackyDB!
═══════════════════════════════════════════════════════════════════════════════
🔹 Project: HackyDB
🔹 Course: CS315 - DATABASE SYSTEMS (6th Semester)
🔹 Institute: Indian Institute of Technology Kanpur
🔹 Instructor: Prof. Arnab Bhattacharya

🧩 Modules: Interpreter | API | Record Manager | Index Manager | Catalog Manager | Buffer Manager (LRU Cache with 4KB Blocks)

👨‍💻 Crafted With Passion By Team HACKY:
   ⚡ Harshit            ⚡ Aditya Gupta
   ⚡ Chayan Kumawat     ⚡ Kushagra Singh
   ⚡ Yash Pratap Singh

💡 Tips:
   • End SQL statements with ';'
   • Type 'help' to see available commands
   • Type 'exit' or 'quit' to close HackyDB
═══════════════════════════════════════════════════════════════════════════════

[HackyDB] Ready to execute your SQL commands. Type 'help' for assistance.

Supported SQL Queries:
-----------------------
1. SELECT * FROM table_name [WHERE column = value [AND ...]]
2. INSERT INTO table_name VALUES (value1, value2, ...)
3. DELETE FROM table_name [WHERE column = value [AND ...]]
4. CREATE DATABASE database_name
5. DROP DATABASE database_name
6. USE database_name
7. CREATE TABLE table_name (column_name TYPE, ..., PRIMARY KEY(column_name))
8. DROP TABLE table_name
9. CREATE INDEX index_name ON table_name(column_name)
10. DROP INDEX index_name
11. EXEC file_name

Note:
- Types: INT, FLOAT, CHAR(n)
- CHAR values must be enclosed in single ('') or double quotes ("")
- WHERE conditions support: =, <, >, <=, >=, <>

HackyDB> SHOW DATABASES;
SQL TYPE: #SHOW DATABASES#
SQL STATEMENT: SHOW DATABASES
DATABASE LIST:
	DB1
	DB2


HackyDB> USE DB2;
SQL TYPE: #USE#
SQL STATEMENT: USE DB2
DB NAME: DB2


HackyDB> SHOW TABLES;
SQL TYPE: #SHOW TABLES#
SQL STATEMENT: SHOW TABLES
CURRENT DATABASE: DB2
TABLE LIST:
	students


HackyDB> SELECT * FROM students;
SQL TYPE: #SELECT#
SQL STATEMENT: SELECT * FROM students
Syntax Error!


HackyDB> select * from students;
SQL TYPE: #SELECT#
SQL STATEMENT: select * from students
TABLE NAME: students
student_idname     departmentcgpa
101      Alice    CSE      9.1
102      Bob      EE       8.7
105      Eve      EE       9.2
104      David    CSE      8.8
109      Ivan     ME       8.2
107      Grace    CSE      9.3
110      Judy     CSE      9.7
106      Frank    ME       8
*****************************************************
KeyCount: 8, NodeCount: 1, Level: 1, Root: 0
----------------------
BlockNum: 0 Count: 8, Parent: -1  IsLeaf:1
Keys: { 101      102      104      105      106      107      109      110       }
Vals: { 0000000 0000001 0000003 0000002 0000007 0000005 0000007 0000008  }
NextLeaf:    -1


HackyDB>
```