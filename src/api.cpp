#include "api.hpp"
#include "stl.hpp"
#include "github/client.hpp"
#include "github/types.hpp"
#include "db/sqlite_store.hpp"
#include "ui_interface/user_list_vm.hpp"
#include "ui_interface/repo_list_vm.hpp"

using mx3::Api;
using json11::Json;

namespace {
    const string USERNAME_KEY      = "username";
    const string LAUNCH_NUMBER_KEY = "launch_number";
}

shared_ptr<mx3_gen::Api>
mx3_gen::Api::create_api(
    const string& root_path,
    const shared_ptr<mx3_gen::EventLoop>& ui_thread,
    const shared_ptr<Http> & http_impl,
    const shared_ptr<ThreadLauncher> & launcher
) {
    const auto ui_runner = make_shared<mx3::EventLoopRef>(ui_thread);
    const auto bg_runner = make_shared<mx3::EventLoopCpp>(launcher);
    return make_shared<mx3::Api>(root_path, ui_runner, bg_runner, http_impl);
}

Api::Api(
    const string& root_path,
    const shared_ptr<SingleThreadTaskRunner>& ui_runner,
    const shared_ptr<SingleThreadTaskRunner>& bg_runner,
    const shared_ptr<mx3_gen::Http>& http_client
) :
    // todo this needs to use a fs/path abstraction (not yet built)
    m_db { std::make_unique<mx3::SqliteStore>(root_path + "/kv.sqlite") },
    m_ui_thread {ui_runner},
    m_bg_thread {bg_runner},
    m_bg_http {http_client, m_bg_thread}
{
    m_sqlite = mx3::sqlite::Db::open(root_path + "/example.sqlite");
    _setup_db();
    m_read_db = mx3::sqlite::Db::open(root_path + "/example.sqlite");

    auto j_launch_number = m_db->get(LAUNCH_NUMBER_KEY);
    size_t launch = 0;
    if (j_launch_number.is_number()) {
        launch = j_launch_number.number_value() + 1;
    }
    m_db->set("launch_number", static_cast<double>(launch));
}

bool
Api::has_user() {
    return !m_db->get(USERNAME_KEY).is_null();
}

string
Api::get_username() {
    return m_db->get(USERNAME_KEY).string_value();
}

void
Api::set_username(const string& username) {
    m_db->set(USERNAME_KEY, username);
}

string
Api::get_selected_repos_url() {
    return m_selected_repos_url;
}

void
Api::set_selected_repos_url(const string &repos_url) {
    m_selected_repos_url = repos_url;
}

int64_t
Api::get_selected_user_id() {
    return m_selected_user_id;
}

void
Api::set_selected_user_id(int64_t user_id) {
    m_selected_user_id = user_id;
}

shared_ptr<mx3_gen::UserListVmHandle>
Api::observer_user_list() {
    return make_shared<mx3::UserListVmHandle>(m_read_db, m_bg_http, m_ui_thread, m_bg_thread);
}

shared_ptr<mx3_gen::RepoListVmHandle>
Api::observer_repo_list() {
    return make_shared<mx3::RepoListVmHandle>(m_read_db, m_bg_http, m_selected_repos_url, m_selected_user_id, m_ui_thread, m_bg_thread);
}

void
Api::_setup_db() {
    vector<string> setup_commands  {
        "CREATE TABLE IF NOT EXISTS `github_users` ("
            "`login` TEXT, "
            "`id` INTEGER, "
            "`avatar_url` TEXT, "
            "`gravatar_id` TEXT, "
            "`url` TEXT, "
            "`html_url` TEXT, "
            "`followers_url` TEXT, "
            "`following_url` TEXT, "
            "`gists_url` TEXT, "
            "`starred_url` TEXT, "
            "`subscriptions_url` TEXT, "
            "`organizations_url` TEXT, "
            "`repos_url` TEXT, "
            "`events_url` TEXT, "
            "`received_events_url` TEXT, "
            "`type` TEXT, "
            "`site_admin` TEXT, "
            "PRIMARY KEY(id)"
        ");"
    };
    vector<string> setup_commands2 {
        "CREATE TABLE IF NOT EXISTS `github_user_repos` ("
        "`id` INTEGER, "
        "`owner_id` INTEGER, "
        "`name` TEXT, "
        "`repo_url` TEXT, "
        "`stargazers_count` INTEGER, "
        "`watchers_count` INTEGER, "
        "`language` TEXT, "
        "PRIMARY KEY(id), "
        "FOREIGN KEY(owner_id) REFERENCES github_users(id)"
        ");"
    };
    for (const auto& cmd : setup_commands) {
        m_sqlite->exec(cmd);
    }
    for (const auto& cmd : setup_commands2) {
        m_sqlite->exec(cmd);
    }
    m_sqlite->enable_wal();
}

string
Api::get_foo() {
	return "bar";
}
