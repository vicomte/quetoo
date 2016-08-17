/*
 * Copyright(c) 1997-2001 id Software, Inc.
 * Copyright(c) 2002 The Quakeforge Project.
 * Copyright(c) 2006 Quetoo.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <assert.h>

#include "ServersTableView.h"

#include "client.h"

extern cl_static_t cls;

#define _Class _ServersTableView

static TableColumn *_hostname;
static TableColumn *_name;
static TableColumn *_gameplay;
static TableColumn *_players;
static TableColumn *_ping;

#pragma mark - TableViewDataSource

/**
 * @see TableViewDataSource::numberOfRows
 */
static size_t numberOfRows(const TableView *tableView) {
	return g_list_length(cls.servers);
}

/**
 * @see TableViewDataSource::valueForColumnAndRow
 */
static ident valueForColumnAndRow(const TableView *tableView, const TableColumn *column, int row) {

	cl_server_info_t *server = g_list_nth_data(cls.servers, row);
	assert(server);

	if (column == _hostname) {
		return server->hostname;
	} else if (column == _name) {
		return server->name;
	} else if (column == _gameplay) {
		return server->gameplay;
	} else if (column == _players) {
		return &server->clients;
	} else if (column == _ping) {
		return &server->ping;
	}

	return NULL;
}

#pragma mark - TableViewDelegate

/**
 * @see TableViewDelegate::cellForColumnAndRow
 */
static TableCellView *cellForColumnAndRow(const TableView *tableView, const TableColumn *column, int row) {

	cl_server_info_t *server = g_list_nth_data(cls.servers, row);
	assert(server);

	TableCellView *cell = $(alloc(TableCellView), initWithFrame, NULL);

	if (column == _hostname) {
		$(cell->text, setText, server->hostname);
	} else if (column == _name) {
		$(cell->text, setText, server->name);
	} else if (column == _gameplay) {
		$(cell->text, setText, server->gameplay);
	} else if (column == _players) {
		$(cell->text, setText, va("%d/%d", server->clients, server->max_clients));
	} else if (column == _ping) {
		$(cell->text, setText, va("%3d", server->ping));
	}

	return cell;
}

#pragma mark - View

/**
 * @see View::updateBindings
 */
static void updateBindings(View *self) {

	super(View, self, updateBindings);

	$((TableView *) self, reloadData);
}

#pragma mark - ServersTableView

/**
 * @fn ServersTableView *ServersTableView::initWithFrame(ServersTableView *self, const SDL_Rect *frame)
 *
 * @memberof ServersTableView
 */
static ServersTableView *initWithFrame(ServersTableView *self, const SDL_Rect *frame) {
	
	self = (ServersTableView *) super(TableView, self, initWithFrame, frame);
	if (self) {
		
		self->tableView.dataSource.numberOfRows = numberOfRows;
		self->tableView.dataSource.valueForColumnAndRow = valueForColumnAndRow;

		self->tableView.delegate.cellForColumnAndRow = cellForColumnAndRow;

		$((TableView *) self, addColumn, _hostname);
		$((TableView *) self, addColumn, _name);
		$((TableView *) self, addColumn, _gameplay);
		$((TableView *) self, addColumn, _players);
		$((TableView *) self, addColumn, _ping);

		$((TableView *) self, reloadData);
	}
	
	return self;
}

#pragma mark - Class lifecycle

/**
 * @see Class::initialize(Class *)
 */
static void initialize(Class *clazz) {

	((ViewInterface *) clazz->interface)->updateBindings = updateBindings;

	((ServersTableViewInterface *) clazz->interface)->initWithFrame = initWithFrame;

	_hostname = $(alloc(TableColumn), initWithIdentifier, "Hostname");
	_name = $(alloc(TableColumn), initWithIdentifier, "Map");
	_gameplay = $(alloc(TableColumn), initWithIdentifier, "Gameplay");
	_players = $(alloc(TableColumn), initWithIdentifier, "Players");
	_ping = $(alloc(TableColumn), initWithIdentifier, "Ping");

	_hostname->width = 300, _name->width = 100, _gameplay->width = 100, _players->width = 80, _ping->width = 80;
}

/**
 * @see Class::destroy(Class *)
 */
static void destroy(Class *clazz) {

	release(_hostname);
	release(_name);
	release(_gameplay);
	release(_players);
	release(_ping);
}

Class _ServersTableView = {
	.name = "ServersTableView",
	.superclass = &_TableView,
	.instanceSize = sizeof(ServersTableView),
	.interfaceOffset = offsetof(ServersTableView, interface),
	.interfaceSize = sizeof(ServersTableViewInterface),
	.initialize = initialize,
	.destroy = destroy
};

#undef _Class

