/**********************************************************************
 * Noriko - cross-platform 2-D role-playing game (RPG) game engine    *
 *          for desktop and mobile console platforms                  *
 *                                                                    *
 * (c) 2024 TophUwO <tophuwo01@gmail.com>. All rights reserved.       *
 *                                                                    *
 * The source code is licensed under the Apache License 2.0. Refer    *
 * to the LICENSE file in the root directory of this project. If this *
 * file is not present, visit                                         *
 *     https://www.apache.org/licenses/LICENSE-2.0                    *
**********************************************************************/

/*
 * sql_assetdb.sql
 * defines the formal schema for the asset database
 */


PRAGMA foreign_keys = OFF;
PRAGMA user_version = 1;


/*
 * table assets
 * defines the assets that are part of the game
 */
CREATE TABLE assets(
    uuid BLOB,                  -- unique identifer, saved as a literal 16-byte integer
    type INT          NOT NULL, -- type (integral)
    name VARCHAR(128) NOT NULL, -- name, up to 128 characters
    path TEXT         NOT NULL, -- path string, using '/' as separator
    docs TEXT,                  -- (optional) documentation string

    PRIMARY KEY (uuid)
);

/*
 * table dependencies
 * defines the dependency graph
 */
CREATE TABLE dependencies(
    depender BLOB NOT NULL, -- UUID of the asset that depends on 'dependee'
    dependee BLOB NOT NULL, -- UUID of the asset that is being depended on

    FOREIGN KEY (depender) REFERENCES assets(uuid),
    FOREIGN KEY (dependee) REFERENCES assets(uuid),
    UNIQUE      (depender, dependee),
    CHECK       (depender != dependee)
);


PRAGMA foreign_keys = ON;


