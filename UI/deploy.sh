#!/bin/bash

rsync -v -e ssh --files-from=rsync.txt . gregory@173.255.227.132:/home/gregory/www-root/potions/
