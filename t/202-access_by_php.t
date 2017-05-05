# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__

=== TEST 1: linked access and content
--- config
    location /access-content {
        access_by_php '
            $var = "var access\n";
            echo "running access\n";
        ';
        content_by_php '
            echo "running content\n";
            echo $var;
        ';
    }
--- request
GET /access-content
--- response_body
running access
running content
var access



=== TEST 2: linked access-content, access block
--- config
    location /access-content {
        access_by_php '
            echo "running access\n";
            ngx::_exit(NGX_OK);
        ';
        content_by_php '
            echo "running content\n";
        ';
    }
--- request
GET /access-content
--- response_body
running access


