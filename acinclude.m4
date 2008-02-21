AC_DEFUN([GNOME_GHTTP_CHECK],[
	GHTTP_LIB=
	AC_CHECK_FUNC(connect,,[
	  AC_CHECK_LIB(socket,connect,
		GHTTP_LIB="-lsocket $GHTTP_LIB",,$GHTTP_LIB)])
	AC_CHECK_FUNC(gethostbyname,,[
 	  AC_CHECK_LIB(nsl,gethostbyname,
		GHTTP_LIB="-lnsl $GHTTP_LIB",,$GHTTP_LIB)])
	AC_CHECK_LIB(ghttp, ghttp_request_new, 
		GHTTP_LIB="-lghttp $GHTTP_LIB",GHTTP_LIB="",$GHTTP_LIB, ghttp_found=yes)

	AC_SUBST(GHTTP_LIB)
	AC_PROVIDE([GNOME_GHTTP_CHECK])
])
