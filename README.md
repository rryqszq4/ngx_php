ngx_php
======

[![Join the chat at https://gitter.im/rryqszq4/ngx_php](https://badges.gitter.im/rryqszq4/ngx_php.svg)](https://gitter.im/rryqszq4/ngx_php?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Build Status](https://travis-ci.org/rryqszq4/ngx_php.svg?branch=master)](https://travis-ci.org/rryqszq4/ngx_php)

Embedded php scripting language for nginx-module.  
[php7-nginx-module](https://github.com/rryqszq4/php7-nginx-module) for php7.  
QQ group：558795330

Features
--------
* Load php.ini config file
* Global variable support $_GET, $_POST, $_COOKIE, $_SERVER, $_FILES, $_SESSION...
* PHP script code and file execute
* RFC 1867 protocol file upload
* PHP error reporting output
* Support PECL PHP extension

Requirement
-----------
- PHP 5.3.*  
PHP 5.4.*  
PHP 5.5.*  
PHP 5.6.*
- nginx-1.4.7  
nginx-1.6.3  
nginx-1.8.1  
nginx-1.9.15

Installation
-------
```sh
git clone https://github.com/rryqszq4/ngx_php.git
cd ngx_php

wget 'http://nginx.org/download/nginx-1.6.3.tar.gz'
tar -zxvf nginx-1.6.3.tar.gz
cd nginx-1.6.3

export PHP_BIN=/path/to/php/bin
export PHP_INC=/path/to/php/include/php
export PHP_LIB=/path/to/php/lib

./configure --user=www --group=www \
			--prefix=/path/to/nginx \
			--with-ld-opt="-Wl,-rpath,$PHP_LIB" \
			--add-module=/path/to/ngx_php/dev/ngx_devel_kit \
			--add-module=/path/to/ngx_php
```

Synopsis
--------

```nginx
user www www;
worker_processes  4;

events {
    worker_connections  1024;
}

http {
    include       mime.types;
    default_type  application/octet-stream;

    keepalive_timeout  65;
	
    client_max_body_size 10m;   
    client_body_buffer_size 4096k;

    php_ini_path /usr/local/php/etc/php.ini;

	server {
        listen       80;
        server_name  localhost;
	
		location /php {
			content_by_php '
				echo "hello ngx_php";
			';
		}
	}
}
```

Framework conf
--------------

**wordpress (多入口模式):**

```nginx
server {
	listen 80;
	server_name	wordpress-sample.com;
	
	location ~ \.php$ {
        root   /home/www/wordpress;
        content_by_php "
            require_once('/home/www/wordpress'.$_SERVER['DOCUMENT_URI']);
        ";
    }
}
```
**yaf & yii (单一入口模式):**
```nginx
server {
    listen 80;
	server_name	yaf-sample.com;
	access_log	logs/yaf-sample.com.access.log;

	root /home/www/yaf-sample;
	index index.php index.html;
	
	location /favicon.ico {
		log_not_found off;
	}

	location / {
		try_files $uri $uri/ /index.php$is_args$args;
	}

	location ~ \.php$ {
		content_by_php '
			header("Content-Type: text/html;charset=UTF-8");
			require_once("/home/www/yaf-sample/index.php");
		';
	}
}  
```

Test
----
Using the perl of [Test::Nginx](https://github.com/openresty/test-nginx) module to testing, searching and finding out problem in ngx_php. 

```sh
cd /path/to/ngx_php
export PATH=/path/to/nginx/sbin:$PATH
prove -r t
```
Test result:

```sh
t/001-hello.t .... ok   
t/002-ini.t ...... ok   
t/003-_GET.t ..... ok   
t/004-_POST.t .... ok   
t/005-_SERVER.t .. ok   
t/006-_COOKIE.t .. ok   
t/007-_FILES.t ... ok   
t/008-error.t .... ok
t/009-session.t .. ok     
All tests successful.
Files=9, Tests=20,  2 wallclock secs ( 0.03 usr  0.03 sys +  1.20 cusr  0.31 csys =  1.57 CPU)
Result: PASS
```

Directives
----------

#### php_ini_path
**syntax:** *php_ini_path &lt;php.ini file path&gt;*  
**context:** *http*  
**phase:** *loading-config*  

加载php配置文件
```nginx
php_ini_path /usr/local/php/etc/php.ini;
```

#### init_by_php
**syntax:** *init_by_php &lt;php script code&gt;*  
**context:** *http*  
**phase:** *loading-config*

#### init_by_php_file
**syntax:** *init_by_php_file &lt;php script file&gt;*  
**context:** *http*  
**phase:** *loading-config*

#### rewrite_by_php
**syntax:** *rewrite_by_php &lt;php script code&gt;*  
**context:** *http, server, location, location if*  
**phase:** *rewrite*

#### rewrite_by_php_file
**syntax:** *rewrite_by_php_file &lt;php script file&gt;*  
**context:** *http, server, location, location if*  
**phase:** *rewrite*

#### access_by_php
**syntax:** *access_by_php &lt;php script code&gt;*  
**context:** *http, server, location, location if*  
**phase:** *access*

#### access_by_php_file
**syntax:** *access_by_php_file &lt;php script file&gt;*  
**context:** *http, server, location, location if*  
**phase:** *access*

#### content_by_php
**syntax:** *content_by_php &lt;php script code&gt;*  
**context:** *http, server, location, location if*  
**phase:** *content*

#### content_by_php_file
**syntax:** *content_by_php_file &lt;php script file&gt;*  
**context:** *http, server, location, location if*  
**phase:** *content*

#### content_async_by_php
**syntax:** *content_async_by_php &lt;php script code&gt;*  
**context:** *http, server, location, location if*  
**phase:** *content*  

异步的代码方式去执行非阻塞的php代码调用

#### content_sync_by_php
**syntax:** *content_sync_by_php &lt;php script code&gt;*  
**context:** *http, server, location, location if*  
**phase:** *content*  

#### set_by_php
**syntax:** *set_by_php &lt;php script code&gt;*  
**context:** *server, server if, location, location if*  
**phase:** *content*

#### set_run_by_php
**syntax:** *set_run_by_php &lt;php script code&gt;*  
**context:** *server, server if, location, location if*  
**phase:** *content*

#### set_by_php_file
**syntax:** *set_by_php_file &lt;php script file&gt;*  
**context:** *server, server if, location, location if*  
**phase:** *content*

#### set_run_by_php_file
**syntax:** *set_run_by_php_file &lt;php script file&gt;*  
**context:** *server, server if, location, location if*  
**phase:** *content*


Nginx API for php
-----------------

#### ngx_location::capture_async
**syntax:** *ngx_location::capture_async(string $uri, mixed $closure)*  
**context:** *content_async_by_php*  

借助nginx底层强大的subrequest，实现php完全非阻塞的异步代码调用
```php
ngx_location::capture_async('/foo', function($callback = 'callback'){
    echo $callback;
});
```

#### ngx_location::capture_multi_async
**syntax:** *ngx_location::capture_multi_async(array $uri, mixed $closure)*  
**context:** *content_async_by_php*  

和ngx_location::capture_async相似，但是可以支持完全非阻塞的并行异步代码调用
```php
$capture_multi = array(
    '/foo',
    '/bar',
    '/baz'
);
ngx_location::capture_multi_async($capture_multi, function($callback = 'callback'){
    var_dump($callback);
});
```

#### ngx_location::capture
**syntax:** *ngx_location::capture(string $uri)*  
**context:** *content_sync_by_php*  

借助nginx底层强大的subrequest，实现php完全非阻塞调用
```php
$result = ngx_location::capture('/foo');
echo $result;
```

#### ngx_location::capture_multi
**syntax:** *ngx_location::capture_multi(array $uri)*  
**context:** *content_sync_by_php*  

和ngx_location::capture相似，但是可以支持完全非阻塞的并行调用
```php
$capture_multi = array(
    '/foo',
    '/bar',
    '/baz'
);
$result = ngx_location::capture_multi($capture_multi);
var_dump($result);
```


Copyright and License
---------------------
Copyright (c) 2016, rryqszq4 <ngxphp@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
