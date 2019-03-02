ngx_php
========
[![Build Status](https://travis-ci.org/rryqszq4/ngx_php.svg?branch=master)](https://travis-ci.org/rryqszq4/ngx_php)
[![GitHub release](https://img.shields.io/github/release/rryqszq4/ngx_php.svg)](https://github.com/rryqszq4/ngx_php/releases/latest)
[![license](https://img.shields.io/badge/license-BSD--2--Clause-blue.svg)](https://github.com/rryqszq4/ngx_php/blob/master/LICENSE)
[![QQ group](https://img.shields.io/badge/QQ--group-558795330-26bcf5.svg)](https://github.com/rryqszq4/ngx_php)

ngx_php is an extension module of high-performance web server nginx, which implements embedded php script to process nginx location and variables.  

ngx_php draws on the design of [ngx_lua](https://github.com/openresty/lua-nginx-module) and is committed to providing non-blocking web services with significant performance advantages over php-cgi, mod_php, php-fpm and hhvm.  

ngx_php doesn't want to replace anything, just want to provide a solution.  

Milestones about the project
---------------------------
* [ngx_php5](https://github.com/rryqszq4/ngx_php/tree/ngx_php5) - A legacy version with php5, which records some of my past code practices and is also valuable.
* [ngx_php7](https://github.com/rryqszq4/ngx_php7) - An active branch of development where you can get more fresh details.

What's different with official php
----------------------------------
* Global variable is unsafe in per request
* Static variable of a class is unsafe in per request
* Do not design singleton mode
* The native IO function works fine, but it slows down nginx

Copyright and License
---------------------
ngx_php is licensed under the [BSD-2-Clause](https://github.com/rryqszq4/ngx_php/blob/master/LICENSE) license. 