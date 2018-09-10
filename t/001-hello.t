# vim:set ft= ts=4 sw=4 et fdm=marker:

use Test::Nginx::Socket 'no_plan';

run_tests();

__DATA__
=== TEST 1: hello, ngx_php
This is just a simple demonstration of the
echo directive provided by ngx_php.
--- config
location = /t {
    content_by_php '
        echo "hello ngx_php!";
    ';
}
--- request
GET /t
--- response_body
hello ngx_php!