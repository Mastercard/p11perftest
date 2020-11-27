pipeline {
    agent any

    stages {
		stage ("Build") {
			steps {
				sh "scl enable devtoolset-9 -- ./bootstrap.sh"
				sh "scl enable devtoolset-9 -- ./configure --prefix=${WORKSPACE} --with-boost=${BOOST_HOME} PKG_CONFIG_PATH=${BOTAN_HOME}/lib/pkgconfig:${OPENSSL_HOME}/lib/pkgconfig"
				sh "scl enable devtoolset-9 -- make"
				sh "scl enable devtoolset-9 -- make install"
				//archiveArtifacts artifacts: "**/*.rpm, CHANGELOG.md"
			}
		}
	}
}
