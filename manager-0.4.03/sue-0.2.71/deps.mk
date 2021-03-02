sue_sel.o: sue_sel.cpp sue_sel.hpp
sue_sess.o: sue_sess.cpp sue_sess.hpp sue_sel.hpp
sue_inet.o: sue_inet.cpp sue_inet.hpp sue_sess.hpp sue_sel.hpp
sue_tcps.o: sue_tcps.cpp sue_tcps.hpp sue_sel.hpp sue_inet.hpp \
 sue_sess.hpp
sue_tcpc.o: sue_tcpc.cpp sue_tcpc.hpp sue_inet.hpp sue_sess.hpp \
 sue_sel.hpp
sue_wait.o: sue_wait.cpp sue_wait.hpp sue_sel.hpp
