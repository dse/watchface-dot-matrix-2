web_dir = /var/www/webonastick.com/htdocs/watchfaces/dot-matrix-2/config/

publish:
	ssh dse@webonastick.com 'mkdir -p $(web_dir)' || true
	rsync -av configpage/ dse@webonastick.com:$(web_dir)

