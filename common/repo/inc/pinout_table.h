/*
 * ****************************************************************************
 *  @author      xqyjlj
 *  @file        pinout_table.h
 *  @brief
 *
 * ****************************************************************************
 *  @attention
 *  Licensed under the GNU General Public License v. 3 (the "License");
 *  You may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  Copyright (C) 2023-2023 xqyjlj<xqyjlj@126.com>
 *
 * ****************************************************************************
 *  Change Logs:
 *  Date           Author       Notes
 *  ------------   ----------   -----------------------------------------------
 *  2023-05-28     xqyjlj       initial version
 */

#ifndef COMMON_REPO_CSP_PINOUT_TABLE_H
#define COMMON_REPO_CSP_PINOUT_TABLE_H

#include "qtyaml.h"

class pinout_table final {
public:
    typedef struct
    {
        QString mode;
        QString type;
    } function_t;

    typedef struct
    {
        int                       position;
        QString                   type;
        QMap<QString, function_t> functions;
    } pinout_unit_t;

    typedef QMap<QString, pinout_unit_t *> pinout_t;

public:
    static pinout_t load_pinout(const QString &path);
    static pinout_t load_pinout(const QString &hal, const QString &name);

private:
    typedef QMap<QString, pinout_unit_t> _pinout_t;

private:
    explicit pinout_table();
    ~pinout_table();

    static _pinout_t _load_pinout(const QString &path);
};

Q_DECLARE_METATYPE(pinout_table::pinout_unit_t)
Q_DECLARE_METATYPE(pinout_table::pinout_unit_t *)

namespace YAML {

template <> struct convert<pinout_table::function_t>
{
    static Node encode(const pinout_table::function_t &rhs)
    {
        Node node;
        node.force_insert("Mode", rhs.mode);
        node.force_insert("Type", rhs.type);
        return node;
    }

    static bool decode(const Node &node, pinout_table::function_t &rhs)
    {
        if (!node.IsMap())
            return false;

        if (node.size() == 0)
        {
            rhs.mode = "";
            rhs.type = "";
        }
        else if (node.size() == 2)
        {
            rhs.mode = node["Mode"].as<QString>();
            rhs.type = node["Type"].as<QString>();
        }
        else
        {
            return false;
        }

        return true;
    }
};

template <> struct convert<pinout_table::pinout_unit_t>
{
    static Node encode(const pinout_table::pinout_unit_t &rhs)
    {
        Node node;
        node.force_insert("Position", rhs.position);
        node.force_insert("Type", rhs.type);
        node.force_insert("Functions", rhs.functions);
        return node;
    }

    static bool decode(const Node &node, pinout_table::pinout_unit_t &rhs)
    {
        if (!node.IsMap())
            return false;

        if (node.size() == 2)
        {
            rhs.position  = node["Position"].as<int>();
            rhs.type      = node["Type"].as<QString>();
            rhs.functions = QMap<QString, pinout_table::function_t>();
        }
        else if (node.size() == 3)
        {
            rhs.position  = node["Position"].as<int>();
            rhs.type      = node["Type"].as<QString>();
            rhs.functions = node["Functions"].as<QMap<QString, pinout_table::function_t>>();
        }
        else
        {
            return false;
        }
        return true;
    }
};
}  // namespace YAML

#endif  // COMMON_REPO_CSP_PINOUT_TABLE_H
