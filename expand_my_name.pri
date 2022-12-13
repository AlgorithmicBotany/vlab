#message( expanding name $(PWD) )
contains( MY_LIBS, $${MY_NAME} ) {
	INCLUDEPATH += $$MY_BASE/libs/$$MY_NAME $$MY_BASE/libs/$$MY_NAME/.uic
	DEPENDPATH += $$MY_BASE/libs/$$MY_NAME $$MY_BASE/libs/$$MY_NAME/.uic
	equals( TEMPLATE, app ) {
		POST_TARGETDEPS += $$MY_BASE/.libraries/lib$${MY_NAME}.a
		LIBS +=            $$MY_BASE/.libraries/lib$${MY_NAME}.a
	}
	MY_LIBS -= $$MY_NAME
}

# unset MY_NAME
MY_NAME =
