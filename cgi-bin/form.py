#!/usr/bin/python
#coding=utf-8

# CGI处理模块
import cgi, cgitb
 
# 创建表单实例
form = cgi.FieldStorage()
 
# 获取数据
site_name = form.getvalue('name')
site_url  = form.getvalue('url')
print("Content-type:text/html\n")
print("<html>")
print("<head>")
print("<meta charset= \"utf-8\" />")
print("<title>我的get方法</title>")
print("</head>")
print("<body>")
print("<h2>%s调用地址是: %s</h2>" % (site_name, site_url))
print("</body>")
print("</html>")