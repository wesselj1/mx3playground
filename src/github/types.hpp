#pragma once
#include "stl.hpp"

namespace github {

    // lets put all the "entities" of the github api in here
    struct User {
        string login;
        int64_t id;
        string avatar_url;
        string gravatar_id;
        string url;
        string html_url;
        string followers_url;
        string following_url;
        string gists_url;
        string starred_url;
        string subscriptions_url;
        string organizations_url;
        string repos_url;
        string events_url;
        string received_events_url;
        string type;
        string site_admin;
    };
    
    struct Repo {
        int64_t id;
        int64_t owner_id;
        string name;
        string repo_url;
        int32_t stargazers_count;
        int32_t watchers_count;
        string language;
    };

}

