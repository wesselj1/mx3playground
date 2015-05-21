//
//  repo_list_vm.cpp
//  mx3
//
//  Created by Joey Wessel on 5/19/15.
//
//

#include "repo_list_vm.hpp"
#include "../github/client.hpp"
#include "../sqlite/sqlite.hpp"
#include "../sqlite_query/query_diff.hpp"

#include <iostream>
#include <thread>
using mx3_gen::RepoListVmObserver;
using mx3_gen::RepoListVmCell;

namespace chrono {
    using namespace std::chrono;
}

namespace {
    // TODO this needs to not be stored in github_users actually. We need to add our own table. Also is this table temporary? I can never browse it after or during an app run.
    const string s_list_stmt  { "SELECT `id`, `name`, `repo_url`, `stargazers_count`, `watchers_count`, `language`, `id` FROM `github_user_repos` WHERE `owner_id` = ?1 ORDER BY name;" };
}

namespace mx3 {
    RepoListVm::RepoListVm(const vector<sqlite::Row> &rows, const std::weak_ptr<RepoListVmHandle> &handle/*, const string repo_url*/)
    : m_rows(rows),
    m_handle(handle)
    /*m_repo_url(repo_url)*/
    {}
    
    int32_t
    RepoListVm::count() {
        return static_cast<int32_t>(m_rows.size());
    }
    
    optional<RepoListVmCell>
    RepoListVm::get(int32_t index) {
        if (index < this->count()) {
            return RepoListVmCell {m_rows[index][0].int64_value(), m_rows[index][1].int64_value(), m_rows[index][2].string_value(), m_rows[index][3].string_value(), m_rows[index][4].int_value(), m_rows[index][5].int_value(), m_rows[index][6].string_value()};
        }
        return nullopt;
    }
    
    RepoListVmHandle::RepoListVmHandle(
                                       shared_ptr<mx3::sqlite::Db> db,
                                       const mx3::Http &http,
                                       string repo_url,
                                       int64_t owner_id,
                                       const shared_ptr<SingleThreadTaskRunner> &ui_thread,
                                       const shared_ptr<SingleThreadTaskRunner> &bg_thread
    )
    : m_db(db)
    , m_monitor(sqlite::QueryMonitor::create_shared(m_db))
    , m_list_stmt(m_db->prepare(s_list_stmt))
    , m_http(http)
    , m_observer(nullptr)
    , m_ui_thread(ui_thread)
    , m_bg_thread(bg_thread)
    {
        m_repo_url = repo_url;
        m_owner_id = owner_id;
        m_list_stmt->bind(1, owner_id);
        m_monitor->listen_to_changes([this] () {
            this->_notify_new_data();
        });
    }
    
    void
    RepoListVmHandle::start(const shared_ptr<RepoListVmObserver> &observer) {
        auto db = m_db;
        auto ui_thread = m_ui_thread;
        m_observer = observer;
        
        github::get_user_repos(m_http, m_repo_url, [db, ui_thread, observer] (vector<github::Repo> repos) mutable {
            auto update_stmt = db->prepare("UPDATE `github_user_repos` SET `owner_id` = ?2, `name` = ?3, `repo_url` = ?4, `stargazers_count` = ?5, `watchers_count` = ?6, `language` = ?7 WHERE `id` = ?1;");
            auto insert_stmt = db->prepare("INSERT INTO `github_user_repos` (`id`, `owner_id`, `name`, `repo_url`, `stargazers_count`, `watchers_count`, `language`) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7);");
            sqlite::TransactionStmts transaction_stmts {db};
            sqlite::WriteTransaction guard {transaction_stmts};
            
            for (const auto& repo : repos) {
                update_stmt->reset();
                update_stmt->bind(1, repo.id);
                update_stmt->bind(2, repo.owner_id);
                update_stmt->bind(3, repo.name);
                update_stmt->bind(4, repo.repo_url);
                update_stmt->bind(5, repo.stargazers_count);
                update_stmt->bind(6, repo.watchers_count);
                update_stmt->bind(7, repo.language);
                if (update_stmt->exec() == 0) {
                    insert_stmt->reset();
                    insert_stmt->bind(1, repo.id);
                    insert_stmt->bind(2, repo.owner_id);
                    insert_stmt->bind(3, repo.name);
                    insert_stmt->bind(4, repo.repo_url);
                    insert_stmt->bind(5, repo.stargazers_count);
                    insert_stmt->bind(6, repo.watchers_count);
                    insert_stmt->bind(7, repo.language);
                    insert_stmt->exec();
                }
            }
            guard.commit();
        });
    }
    
    void
    RepoListVmHandle::stop() {
        
    }
    
    void
    RepoListVmHandle::_notify_new_data() {
        // TODO kabbes says this is not thread safe
        
        optional<vector<mx3_gen::ListChange>> diff;
        
        decltype(m_prev_rows) prev_rows;
        std::swap(m_prev_rows, prev_rows);
        auto new_rows = m_list_stmt->exec_query().all_rows();
        
        if (prev_rows) {
            constexpr const size_t ID_POS = 1;
            const auto is_same_entity = [] (const sqlite::Row &a, const sqlite::Row &b) {
                return a[ID_POS] == b[ID_POS];
            };
            const auto less_than = [] (const sqlite::Row &a, const sqlite::Row &b) {
                return a[ID_POS] < b[ID_POS];
            };
            const auto sql_diff = sqlite::calculate_diff(*prev_rows, new_rows, is_same_entity, less_than);
            vector<mx3_gen::ListChange> final_diff;
            final_diff.reserve(sql_diff.size());
            for (const auto &c : sql_diff) {
                final_diff.push_back({c.from_index, c.to_index});
            }
            diff = std::move(final_diff);
        }
        
        m_prev_rows = std::move(new_rows);
        const std::weak_ptr<RepoListVmHandle> weak_self = shared_from_this();
        m_ui_thread->post([diff = std::move(diff), weak_self, observer = m_observer, new_rows = *m_prev_rows] () {
            // TODO from kabbes "make sure to check if this has been stopped"
            observer->on_update(diff, make_shared<RepoListVm>(new_rows, weak_self));
        });
    }
}
