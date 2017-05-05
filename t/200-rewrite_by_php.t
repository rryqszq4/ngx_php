# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__

=== TEST 1: linked rewrite and content
--- config
    location /rewrite-content {
        rewrite_by_php '
            $var_rewrite = "var rewrite\n";
            echo "running rewrite\n";
        ';
        content_by_php '
            echo "running content\n";
            echo $var_rewrite;
        ';
    }
--- request
GET /rewrite-content
--- response_body
running rewrite
running content
var rewrite



=== TEST 2: linked rewrite-content, rewrite block
--- config
    location /rewrite-content {
        rewrite_by_php '
            echo "running rewrite\n";
            ngx::_exit(NGX_OK);
        ';
        content_by_php '
            echo "running content\n";
        ';
    }
--- request
GET /rewrite-content
--- response_body
running rewrite


