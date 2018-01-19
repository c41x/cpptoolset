# pull first
git pull origin master
git pull git master

# push to repositories
git push origin master
git push git master
cd ../engine
git pull origin master
git push origin master

# generate statistics (gitstats required)
cd ..
gitstats .git /tmp/gitstats/granite
cd engine
gitstats .git /tmp/gitstats/granite-engine

# launch stats in firefox
firefox /tmp/gitstats/granite/index.html
firefox /tmp/gitstats/granite-engine/index.html
