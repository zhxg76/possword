/*
 * 郑翔 202010 诗词数据库操作函数
 */

#include <stdio.h>
#include <unistd.h>
#include <sqlite3.h>

#define POEMDB "poem.db"
#define POEMDBPATH "/opt/possword/poem.db"

#define SQLITE_OPEN_MODE SQLITE_OPEN_FULLMUTEX|SQLITE_OPEN_SHAREDCACHE|SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE

#define DFT_BUSY_TIMEOUT 100*1000
int db_busy_callback(void *data, int count)
{
	if (count >= 10) {
		return 0;
	}
	usleep(DFT_BUSY_TIMEOUT);
	return SQLITE_ERROR;
};

sqlite3* poem_db = NULL;

int poems_num = 0;
static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	poems_num = atoi(argv[0]);
	return 0;
}
void get_poems_num(sqlite3 *db)
{
	int rc = 0;
	char *errmsg = NULL;

	rc = sqlite3_exec(db, "SELECT count(*) FROM poeminfo;", callback, 0, &errmsg);
	if (rc != SQLITE_OK) {
		printf("get_poems_num fail: %s\n", errmsg);
		sqlite3_free(errmsg);
	}
}

sqlite3* connectDb(char *dbname, const char *crt_tbl_sql)
{
	int rc = 0;
	sqlite3 *db = NULL;

	if (access(dbname, F_OK) == 0) {
		rc = sqlite3_open_v2(dbname, &db, SQLITE_OPEN_MODE, NULL);
		if (rc == SQLITE_OK) {
			rc = sqlite3_exec(db, "SELECT count(*) FROM sqlite_master;", NULL, NULL, NULL);
			if (rc == SQLITE_OK) {
				printf("open db %s ok\n", dbname);
				return db;
			}
		}
		if (db) {
			sqlite3_close_v2(db);
		}
		printf("open db %s fail: %s\n", dbname, sqlite3_errstr(rc));
		return NULL;
	}

	if (errno != ENOENT) {
		printf("open db %s fail: %s\n", dbname, strerror(errno));
		return NULL;
	}

	if (!crt_tbl_sql) {
		return NULL;
	}

	/* 创建新数据库 */
	rc = sqlite3_open_v2(dbname, &db, SQLITE_OPEN_MODE, NULL);
	if (rc == SQLITE_OK) {
		rc = sqlite3_exec(db, crt_tbl_sql, NULL, NULL, NULL);
		if (rc == SQLITE_OK) {
			printf("create db %s success\n", dbname);
			return db;
		}
		printf("create table in db %s fail: %s\n", dbname, sqlite3_errstr(rc));
	}

	printf("create db %s fail: %s\n", dbname, sqlite3_errstr(rc));
	if (db) {
		sqlite3_close_v2(db);
	}
	return NULL;
}

const char crt_poem_tbl_sql[1024] =
{
    "CREATE TABLE IF NOT EXISTS poeminfo( "
    "id integer PRIMARY KEY AUTOINCREMENT,"
    "poem varchar(64) UNIQUE);"
};

const char* poem_new_sql = "INSERT INTO poeminfo VALUES(NULL,?);";
sqlite3_stmt* poem_new_stmt = NULL;

static sqlite3* db_init(char *dbfile)
{
	sqlite3* db = connectDb(dbfile, crt_poem_tbl_sql);
	if (db == NULL) {
		return NULL;
	}

	sqlite3_busy_handler(db, db_busy_callback, NULL );
	sqlite3_prepare_v2(db, poem_new_sql, -1, &poem_new_stmt, NULL);

	return db;
}

void db_release(sqlite3* db)
{
	if (db == NULL) {
		return;
	}

	sqlite3_finalize(poem_new_stmt);
	sqlite3_close_v2(db);
}

int get_poem(sqlite3* db, int id, char *poem)
{
	int rc = 0;
	int nrow = 0, ncolumn = 0;
	char **azResult = NULL;
	char buf[1024] = {0};

	snprintf(buf, sizeof(buf), "SELECT id,poem FROM poeminfo WHERE id='%d';", id);
	rc = sqlite3_get_table(db, buf, &azResult, &nrow, &ncolumn, NULL);
	if (rc != SQLITE_OK) {
		sqlite3_free_table(azResult);
		printf("get poem %d from db fail: %s\n", id, sqlite3_errstr(rc));
		return -1;
	}

	strncpy(poem, azResult[ncolumn+1], 63);
	sqlite3_free_table(azResult);
	return 0;
}

int check_poem(sqlite3* db, char *poem)
{
	int rc = 0;
	int nrow = 0, ncolumn = 0;
	char **azResult = NULL;
	char buf[1024] = {0};

	snprintf(buf, sizeof(buf), "SELECT id,poem FROM poeminfo WHERE poem='%s';", poem);
	rc = sqlite3_get_table(db, buf, &azResult, &nrow, &ncolumn, NULL);
	sqlite3_free_table(azResult);
	if (rc != SQLITE_OK) {
		printf("query %s from db fail: %s\n", poem, sqlite3_errstr(rc));
		return -1;
	}

	if (nrow == 0) {
		return 0;
	}

	return 1;
}

int insert_poem(sqlite3* db, char *poem)
{
	int rc = 0;

	rc = check_poem(db, poem);
	if (rc < 0) {
		return -1;
	}

	if (rc == 1) {
		return -1;
		printf("poem %s already in db\n", poem);
		return 0;
	}

	sqlite3_reset(poem_new_stmt);
	sqlite3_bind_text(poem_new_stmt,1,poem,-1,SQLITE_STATIC);
	if ((rc = sqlite3_step(poem_new_stmt)) != SQLITE_DONE) {
		printf("insert new poem %s fail: %s(%d)\n", poem, sqlite3_errstr(rc), rc);
		return -1;
	}

	return 1;
}
