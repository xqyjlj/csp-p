/*
 * ****************************************************************************
 *  @author      xqyjlj
 *  @file        graphicsitem_pin.h
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
 *  2023-06-07     xqyjlj       initial version
 */

#ifndef CSP_GRAPHICSITEM_PIN_H
#define CSP_GRAPHICSITEM_PIN_H

#include <QFont>
#include <QFontMetrics>
#include <QGraphicsItem>
#include <QMenu>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "interface_graphicsitem_pin.h"
#include "pinout_table.h"
#include "project.h"

class graphicsitem_pin : public interface_graphicsitem_pin {
    Q_OBJECT
public:
    enum direction
    {
        TOP = 0,
        BOTTOM,
        LEFT,
        RIGHT
    };

public:
    explicit graphicsitem_pin(qreal width, qreal height);
    ~graphicsitem_pin() override;

    /**
     * @brief set pin direction
     * @param direct: pin direction
     */
    void set_direction(enum direction direct);

    /**
     * @brief set pinout unit
     * @param unit: pinout unit
     */
    void set_pinout_unit(pinout_table::pinout_unit_t *unit);

    /**
     * @brief set pin selected status
     * @param selected: selected status
     */
    void set_selected(bool selected);

    /**
     * @brief set pin name
     * @param name: pin name
     */
    void set_name(const QString &name);

signals:
    void signal_property_changed(QGraphicsItem *item);

private slots:

    /**
     * @brief menu triggered callback
     * @param action: triggered action
     */
    void menu_triggered_callback(QAction *action);

    /**
     * @brief pin property changed callback
     * @param property: property name
     * @param name: pin name
     * @param old_value: old value
     * @param new_value: new value
     */
    void pin_property_changed_callback(const QString  &property,
                                       const QString  &name,
                                       const QVariant &old_value,
                                       const QVariant &new_value);

private:
    const QColor default_color  = QColor(185, 196, 202);
    const QColor power_color    = QColor(255, 246, 204);
    const QColor other_color    = QColor(187, 204, 0);
    const QColor selected_color = QColor(0, 204, 68);

    qreal                        _width;
    qreal                        _height;
    enum direction               _direction   = LEFT;
    pinout_table::pinout_unit_t *_pinout_unit = nullptr;
    bool                         _selected    = false;
    QString                      _name;
    QFont                       *_font;
    QFontMetrics                *_font_metrics;
    QString                      _comment;
    QString                      _function;
    project                     *_project_instance;

    QMenu   *_menu                    = nullptr;
    QAction *_previous_checked_action = nullptr;
    QAction *_current_checked_action  = nullptr;

protected:
    /**
     * @brief defines the outer bounds of the item as a rectangle;
     * @return bounding rectangle
     */
    QRectF boundingRect() const override;

    /**
     * @brief the shape of this item as a QPainterPath in local coordinates
     * @return shape
     */
    QPainterPath shape() const override;

    /**
     * @brief paint the contents of an item in local coordinates
     * @param painter: painter
     * @param option: style option
     * @param widget: widget
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif  // CSP_GRAPHICSITEM_PIN_H
