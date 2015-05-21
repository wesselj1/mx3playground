#include "client.hpp"
#include <json11/json11.hpp>

using github::Client;

using json11::Json;
#include <iostream>
using std::cout;
using std::endl;

namespace {
    const string BASE_URL = "https://api.github.com";
}

github::User
github::parse_user(const json11::Json& data) {
    github::User user;
    user.login               = data["login"].string_value();
    user.id                  = data["id"].number_value();
    user.avatar_url          = data["avatar_url"].string_value();
    user.gravatar_id         = data["gravatar_id"].string_value();
    user.url                 = data["url"].string_value();
    user.html_url            = data["html_url"].string_value();
    user.followers_url       = data["followers_url"].string_value();
    user.following_url       = data["following_url"].string_value();
    user.gists_url           = data["gists_url"].string_value();
    user.starred_url         = data["starred_url"].string_value();
    user.subscriptions_url   = data["subscriptions_url"].string_value();
    user.organizations_url   = data["organizations_url"].string_value();
    user.repos_url           = data["repos_url"].string_value();
    user.events_url          = data["events_url"].string_value();
    user.received_events_url = data["received_events_url"].string_value();
    user.type                = data["type"].string_value();
    user.site_admin          = data["site_admin"].bool_value();
    if (user.avatar_url.empty()) {
        user.avatar_url = "";
    }
    return user;
}

github::Repo
github::parse_repo(const json11::Json &data) {
    github::Repo repo;
    repo.id = data["id"].number_value();
    repo.owner_id = data["owner"]["id"].number_value();
    repo.name = data["name"].string_value();
    repo.repo_url = data["html_url"].string_value();
    repo.stargazers_count = data["stargazers_count"].number_value();
    repo.watchers_count = data["watchers_count"].number_value();
    repo.language = data["language"].string_value();
    return repo;
}

Client::Client(mx3::Http http_client) : m_http {http_client} {}

// todo error handling?
void
github::get_users(mx3::Http http, optional<uint64_t> since, function<void(vector<github::User>)> callback) {
    string url = BASE_URL + "/users";
    if (since) {
        url += "?since=" + std_patch::to_string(*since);
    }

    http.get(url, [callback] (mx3::HttpResponse resp) {
        if (resp.error) {
            return;
        }

        vector<github::User> users;
        string error;
        auto json_response = Json::parse(resp.data, error);
        if (!error.empty()) {
            // there was an error
            // fail somehow
        } else {
            if (json_response.is_array()) {
                for (const auto& item : json_response.array_items()) {
                    users.emplace_back( github::parse_user(item) );
                }
            }
        }
        callback(users);
    });
}

void
github::get_user_repos(mx3::Http http, string repos_url, function<void (vector<github::Repo>)> callback) {
    http.get(repos_url, [callback] (mx3::HttpResponse resp) {
        if (resp.error) {
            return;
        }
        
        vector<github::Repo> repos;
        string error;
        auto json_response = Json::parse(resp.data, error);
        if (!error.empty()) {
            // there was an error
            // fail somehow
        } else {
            if (json_response.is_array()) {
                for (const auto &item : json_response.array_items()) {
                    repos.emplace_back( github::parse_repo(item) );
                }
            }
        }
        callback(repos);
    });
}

void
Client::get_users(optional<uint64_t> since, function<void(vector<github::User>)> callback) {
    github::get_users(m_http, since, callback);
}

void
Client::get_user_repos(string repos_url, function<void (vector<github::Repo>)> callback) {
    github::get_user_repos(m_http, repos_url, callback);
}
