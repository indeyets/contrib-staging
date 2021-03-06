/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2004,2008 Oracle.  All rights reserved.
 *
 * $Id: DbEnvPoolTest.java,v 1.13 2008/07/01 03:21:04 tao Exp $
 */

package com.sleepycat.je.dbi;

import java.io.File;
import java.io.IOException;

import junit.framework.TestCase;

import com.sleepycat.je.Environment;
import com.sleepycat.je.EnvironmentConfig;
import com.sleepycat.je.util.TestUtils;

public class DbEnvPoolTest extends TestCase {

    private File envHome = new File(System.getProperty(TestUtils.DEST_DIR));

    public DbEnvPoolTest() {
    }

    public void setUp()
        throws IOException {

        TestUtils.removeLogFiles("Setup", envHome, false);
    }

    public void tearDown()
        throws Exception {

	TestUtils.removeLogFiles("TearDown", envHome, false);
    }

    public void testCanonicalEnvironmentName ()
        throws Throwable {

        try {
            File file2 = new File("build/test/classes");

            /* Create an environment. */
            EnvironmentConfig envConfig = TestUtils.initEnvConfig();
            envConfig.setAllowCreate(true);
            Environment envA = new Environment(envHome, envConfig);

            /* Look in the environment pool with the relative path name. */
            EnvironmentImpl envImpl =
                DbEnvPool.getInstance().getEnvironment
                    (file2, TestUtils.initEnvConfig(),
                     false /*checkImmutableParams*/,
                     false /*openIfNeeded*/,
                     false /*replicationIntended*/);
            /* We should find this file in the pool without opening it. */
            assertNotNull(envImpl);
            envImpl.decReferenceCount();
            envA.close();

        } catch (Throwable t) {
            /* Dump stack trace before trying to tear down. */
            t.printStackTrace();
            throw t;
        }
    }
}
