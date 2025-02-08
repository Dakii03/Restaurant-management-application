from flask import Flask, request, jsonify, send_file
from datetime import datetime
import sqlite3
import hashlib
#import traceback
from threading import Lock
#import qrcode
# import io

app = Flask(__name__)
db_lock = Lock()

def get_db_connection():
    try:
        conn = sqlite3.connect('restaurant.db')
        conn.row_factory = sqlite3.Row
        return conn
    except sqlite3.Error as e:
        print(f"Error connecting to database: {e}")
        return None

def hash_password(password):
    return hashlib.sha256(password.encode()).hexdigest()

@app.route('/open_database', methods=['GET'])
def open_database():
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute("SELECT 1")
        conn.close()

        return jsonify({"status": "success", "message": "Database connection successful"}), 200

    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500

def get_cart_items_from_db(user_id):
    conn = get_db_connection()
    cursor = conn.cursor()

    cursor.execute('''
        SELECT ci.cart_item_id, ci.item_id, ci.quantity, mi.item_name, mi.price
        FROM cart_items ci
        JOIN menuitems mi ON ci.item_id = mi.item_id
        JOIN carts c ON ci.cart_id = c.cart_id
        WHERE c.user_id = ?
    ''', (user_id,))

    items = cursor.fetchall()

    cart_items = []
    for cart_item_id, item_id, quantity, item_name, price in items:
        cart_items.append({
            'cart_item_id': cart_item_id,
            'item_id': item_id,
            'item_name': item_name,
            'quantity': quantity,
            'price': price
        })

    conn.close()
    return cart_items

@app.route('/get_user_role', methods=['POST'])
def get_user_role():
    data = request.json
    username = data.get('username')

    if not username:
        return jsonify({"error": "Username is required"}), 400

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('''
            SELECT role
            FROM users
            WHERE username = ?
        ''', (username,))

        row = cursor.fetchone()
        if row:
            return jsonify({"role_name": row['role']}), 200
        else:
            return jsonify({"error": "User not found"}), 404

    except sqlite3.Error as e:
        return jsonify({"error": str(e)}), 500

    finally:
        if conn:
            conn.close()

@app.route('/register_customer', methods=['POST'])
def register_customer():
    data = request.get_json()

    required_fields = ['username', 'password_hash', 'full_name', 'email', 'phone']
    if not all(field in data for field in required_fields):
        return jsonify({"status": "error", "message": "Missing required fields"}), 400

    username = data['username']
    password_hash = data['password_hash']
    full_name = data['full_name']
    email = data['email']
    phone = data['phone']

    db = get_db_connection()
    if db is None:
        return jsonify({"status": "error", "message": "Database connection failed"}), 500

    try:
        query = """INSERT INTO users (username, password_hash, role, full_name, email, phone, is_staff)
                   VALUES (?, ?, 'customer', ?, ?, ?, 0)"""
        cur = db.cursor()
        cur.execute(query, (username, password_hash, full_name, email, phone))
        user_id = cur.lastrowid

        role_id = 5
        assignment_query = """INSERT INTO role_assignments (user_id, role_id) VALUES (?, ?)"""
        cur.execute(assignment_query, (user_id, role_id))

        db.commit()

        return jsonify({"status": "success", "user_id": user_id}), 201

    except sqlite3.Error as e:
        db.rollback()  # Rollback in case of an error
        return jsonify({"status": "error", "message": str(e)}), 500
    finally:
        db.close()

@app.route('/get_user_id', methods=['POST'])
def get_user_id():
    data = request.get_json()
    username = data.get('username', None)

    if not username:
        return jsonify({"success": False, "message": "Username is required"}), 400

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute("SELECT user_id FROM users WHERE username = ?", (username,))
        result = cursor.fetchone()

        if result:
            user_id = result[0]
            response = jsonify({"success": True, "user_id": user_id})
            #print(user_id)
            return response, 200
        else:
            return jsonify({"success": False, "message": "User not found"}), 404
    except sqlite3.Error as e:
        return jsonify({"success": False, "message": str(e)}), 500
    finally:
        if conn:
            conn.close()

@app.route('/get_item_id', methods=['POST'])
def get_item_id():
    data = request.get_json()
    item_name = data.get('item_name')

    if not item_name:
        return jsonify({"status": "error", "message": "Item name not provided"}), 400

    # Database connection
    db = get_db_connection()
    if db is None:
        return jsonify({"status": "error", "message": "Database connection failed"}), 500

    try:
        cursor = db.cursor()
        cursor.execute("SELECT item_id FROM menuitems WHERE LOWER(item_name) = LOWER(?)", (item_name,))
        result = cursor.fetchone()

        if result:
            return jsonify({"status": "success", "item_id": result[0]}), 200
        else:
            print(f"Item not found: {item_name}")
            return jsonify({"status": "error", "message": "Item not found"}), 404
    except sqlite3.Error as e:
        print(f"Database error: {str(e)}")
        return jsonify({"status": "error", "message": str(e)}), 500
    finally:
        db.close()

@app.route('/menu_items', methods=['GET'])
def menu_items():
    db = get_db_connection()
    if db is None:
        return jsonify({"status": "error", "message": "Database connection failed"}), 500

    try:
        cursor = db.cursor()
        cursor.execute("SELECT item_name, description, price, type, image_url FROM menuitems")
        rows = cursor.fetchall()

        menu_items = []
        for row in rows:
            menu_items.append({
                "item_name": row["item_name"],
                "description": row["description"],
                "price": row["price"],
                "type": row["type"],
                "image_url": row["image_url"]
            })

        return jsonify(menu_items), 200
    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500
    finally:
        db.close()

@app.route('/check_cart', methods=['POST'])
def check_cart():
    data = request.get_json()
    user_id = data.get('user_id')

    if not user_id:
        return jsonify({"error": "user_id is required"}), 400

    conn = get_db_connection()
    cursor = conn.cursor()

    cursor.execute("SELECT cart_id, status FROM carts WHERE user_id = ? ORDER BY created_at DESC LIMIT 1", (user_id,))
    cart = cursor.fetchone()

    if cart is None:
        return jsonify({"cart_id": -1}), 200
    else:
        return jsonify({"cart_id": cart['cart_id'], "status": cart['status']}), 200

@app.route('/create_cart', methods=['POST'])
def create_cart():
    data = request.get_json()
    user_id = data.get('user_id')

    if not user_id:
        return jsonify({"error": "user_id is required"}), 400

    conn = get_db_connection()
    cursor = conn.cursor()

    try:
        cursor.execute(
            "INSERT INTO carts (user_id, created_at) VALUES (?, datetime('now'))",
            (user_id,)
        )
        conn.commit()

        new_cart_id = cursor.lastrowid
        print(f"DEBUG: New cart ID is {new_cart_id}")

        if new_cart_id == 0:
            raise Exception("Failed to retrieve the new cart ID")

        return jsonify({"cart_id": new_cart_id}), 201

    except sqlite3.Error as e:
        return jsonify({"error": str(e)}), 500

    finally:
        conn.close()

@app.route('/add_to_cart', methods=['POST'])
def add_to_cart():
    data = request.get_json()

    cart_id = data.get('cart_id')
    items = data.get('items')

    if not cart_id:
        return jsonify({"error": "cart_id is required"}), 401
    if not items or not isinstance(items, list):
        return jsonify({"error": "items are required and should be a list"}), 402

    conn = get_db_connection()
    cursor = conn.cursor()

    try:
        for item in items:
            item_id = item.get('item_id')
            quantity = item.get('quantity')

            if not item_id or not quantity:
                return jsonify({"error": "Each item must have item_id and quantity"}), 403

            cursor.execute("SELECT quantity FROM cart_items WHERE cart_id = ? AND item_id = ?", (cart_id, item_id))
            existing_item = cursor.fetchone()

            if existing_item:
                new_quantity = existing_item['quantity'] + quantity
                cursor.execute("UPDATE cart_items SET quantity = ? WHERE cart_id = ? AND item_id = ?", (new_quantity, cart_id, item_id))
            else:
                cursor.execute("INSERT INTO cart_items (cart_id, item_id, quantity) VALUES (?, ?, ?)", (cart_id, item_id, quantity))

        conn.commit()
        return jsonify({"message": "Items successfully added/updated in the cart"}), 201

    except sqlite3.Error as e:
        conn.rollback()
        print(f"Error adding items to cart: {e}")
        return jsonify({"error": "An error occurred while adding items to the cart"}), 500

    finally:
        conn.close()

@app.route('/cart_items/<int:user_id>', methods=['GET'])
def get_cart_items(user_id):
    cart_items = get_cart_items_from_db(user_id)

    if not cart_items:
        return jsonify({"items": []}), 200

    return jsonify({"items": cart_items}), 200

@app.route('/remove_cart_item', methods=['POST'])
def remove_cart_item():
    try:
        data = request.get_json()

        if 'user_id' not in data or 'item_id' not in data:
            return jsonify({"status": "error", "message": "Missing user_id or item_id"}), 400

        user_id = data['user_id']
        item_id = data['item_id']

        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('''
            DELETE FROM cart_items
            WHERE cart_id IN (
                SELECT cart_id FROM carts WHERE user_id = ?
            ) AND item_id = ?
        ''', (user_id, item_id))

        if cursor.rowcount == 0:
            return jsonify({"status": "error", "message": "Item not found in the cart"}), 404

        conn.commit()

        return jsonify({"status": "success", "message": "Item removed from cart"}), 200

    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500

    finally:
        conn.close()

@app.route('/update_cart_quantity', methods=['POST'])
def update_cart_quantity():
    data = request.get_json()
    user_id = data.get('user_id')
    item_name = data.get('item_name')
    new_quantity = data.get('quantity')

    if user_id is None or item_name is None or new_quantity is None:
        return jsonify({'error': 'Invalid input, please provide user_id, item_name, and quantity.'}), 400

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('''
            UPDATE cart_items ci
            JOIN menuitems mi ON ci.item_id = mi.item_id
            SET ci.quantity = ?
            WHERE ci.user_id = ? AND mi.item_name = ?
        ''', (new_quantity, user_id, item_name))

        conn.commit()

        if cursor.rowcount == 0:
            return jsonify({'error': 'No matching item found for the given user.'}), 404

        return jsonify({'success': 'Quantity updated successfully.'}), 200

    except Exception as e:
        return jsonify({'error': str(e)}), 500

    finally:
        conn.close()

@app.route('/clear_cart', methods=['POST'])
def clear_cart():
    try:
        data = request.get_json()

        if 'user_id' not in data:
            return jsonify({"status": "error", "message": "User ID not provided"}), 400

        user_id = data['user_id']

        conn = get_db_connection()
        cursor = conn.cursor()

        try:
            cursor.execute('''
                            DELETE FROM cart_items
                            WHERE cart_id IN (
                                SELECT cart_id FROM carts WHERE user_id = ?
                            )
                        ''', (user_id,))

            cursor.execute('''
                DELETE FROM carts
                WHERE user_id = ?
            ''', (user_id,))

            conn.commit()

            return jsonify({"status": "success", "message": "Cart cleared successfully"}), 200
        except sqlite3.Error as e:
            conn.rollback()
            return jsonify({"status": "error", "message": str(e)}), 500
        finally:
            conn.close()
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/clear_cart_items', methods=['POST'])
def clear_cart_items():
    try:
        data = request.get_json()

        if 'user_id' not in data:
            return jsonify({"status": "error", "message": "User ID not provided"}), 400

        user_id = data['user_id']

        conn = get_db_connection()
        cursor = conn.cursor()

        try:
            cursor.execute('''
                DELETE FROM cart_items
                WHERE cart_id IN (
                    SELECT cart_id FROM carts WHERE user_id = ?
                )
            ''', (user_id,))

            conn.commit()

            return jsonify({"status": "success", "message": "Cart cleared successfully"}), 200
        except sqlite3.Error as e:
            conn.rollback()
            return jsonify({"status": "error", "message": str(e)}), 500
        finally:
            conn.close()
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/place_order', methods=['POST'])
def place_order():
    try:
        data = request.get_json()

        if not data:
            return jsonify({"status": "error", "message": "No JSON data provided"}), 400

        user_id = data.get('user_id')
        customer_name = data.get('customer_name')
        role_id = data.get('role_id')
        order_items = data.get('order_items')

        if not user_id:
            return jsonify({"status": "error", "message": "Missing user_id"}), 401
        if not customer_name:
            return jsonify({"status": "error", "message": "Missing customer_name"}), 402
        if not role_id:
            return jsonify({"status": "error", "message": "Missing role_id"}), 403
        if not order_items:
            return jsonify({"status": "error", "message": "Missing order_items"}), 404

        conn = get_db_connection()
        cursor = conn.cursor()

        created_at = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        cursor.execute('''
            INSERT INTO customer_orders (customer_name, user_id, role_id, order_status, created_at)
            VALUES (?, ?, ?, 'Pending', ?)
        ''', (customer_name, user_id, role_id, created_at))
        order_id = cursor.lastrowid

        for item in order_items:
            item_id = item.get('item_id')
            quantity = item.get('quantity')

            if not item_id or not isinstance(quantity, int) or quantity <= 0:
                conn.close()
                return jsonify({"status": "error", "message": f"Invalid item or quantity for item_id {item_id}."}), 400

            cursor.execute('''
                INSERT INTO customer_order_items (order_id, item_id, quantity)
                VALUES (?, ?, ?)
            ''', (order_id, item_id, quantity))

        cursor.execute('''
            UPDATE carts
            SET status = 'not-active'
            WHERE user_id = ? AND status = 'active'
        ''', (user_id,))

        if cursor.rowcount == 0:
            conn.rollback()
            conn.close()
            return jsonify({"status": "error", "message": "Failed to update cart status to not-active."}), 400

        conn.commit()
        conn.close()

        return jsonify({"status": "success", "message": "Order placed successfully"}), 200

    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/get_user_info/<int:user_id>', methods=['GET'])
def get_user_info(user_id):
    # Connect to the SQLite database
    conn = get_db_connection()
    cursor = conn.cursor()

    try:
        cursor.execute('''
            SELECT full_name, email, role, phone
            FROM users
            WHERE user_id = ?
        ''', (user_id,))

        user = cursor.fetchone()

        if user:
            user_info = {
                "full_name": user[0],
                "email": user[1],
                "role": user[2],
                "phone": user[3]
            }
            return jsonify(user_info), 200
        else:
            return jsonify({"error": "User not found"}), 404
    except sqlite3.Error as e:
        return jsonify({"error": str(e)}), 500
    finally:
        conn.close()


@app.route('/user_info_order/<int:user_id>', methods=['GET'])
def get_user_info_order(user_id):
    conn = get_db_connection()
    cursor = conn.cursor()

    cursor.execute('''
        SELECT full_name, role
        FROM users
        WHERE user_id = ?
    ''', (user_id,))

    result = cursor.fetchone()
    conn.close()

    if result:
        full_name, role_name = result['full_name'], result['role']

        role_ids = {
            'administrator': 1,
            'waiter': 2,
            'manager': 3,
            'kitchen_staff': 4,
            'customer': 5
        }

        role_id = role_ids.get(role_name.lower(), None)

        if role_id:
            return jsonify({'full_name': full_name, 'role_id': role_id}), 200
        else:
            return jsonify({'error': 'Role not found'}), 404
    else:
        return jsonify({'error': 'User not found'}), 404


@app.route('/logout', methods=['POST'])
def logout():
    data = request.json
    user_id = data.get('user_id')

    if not user_id:
        return jsonify({"error": "User ID is required"}), 400

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('''
            SELECT r.role_name
            FROM roles r
            JOIN role_assignments ra ON r.role_id = ra.role_id
            WHERE ra.user_id = ?
        ''', (user_id,))
        role = cursor.fetchone()

        if not role:
            return jsonify({"error": "User role not found."}), 402

        if role[0].lower() != 'customer':
            cursor.execute('''
                UPDATE check_in_out
                SET check_out_time = datetime('now')
                WHERE staff_id = (SELECT staff_id FROM staff WHERE user_id = ?)
                AND check_out_time IS NULL
            ''', (user_id,))

            if cursor.rowcount == 0:
                return jsonify({"error": "Failed to update check-out time or no active check-in found."}), 401

        cursor.execute('''
            SELECT 1 FROM sessions
            WHERE expires_at > datetime('now') AND user_id = ?
        ''', (user_id,))

        if cursor.fetchone() is not None:
            cursor.execute('''
                DELETE FROM sessions
                WHERE expires_at > datetime('now') AND user_id = ?
            ''', (user_id,))

        conn.commit()

        return jsonify({"message": "Logout successful."}), 200

    except sqlite3.Error as e:
        print(f"Database error: {str(e)}")
        return jsonify({"error": "Internal server error. Please try again later."}), 500

    finally:
        if conn:
            conn.close()

@app.route('/check_in', methods=['POST'])
def check_in():
    data = request.get_json()
    user_id = data.get('user_id')

    if not user_id:
        return jsonify({"status": "error", "message": "User ID is required"}), 400

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('SELECT staff_id FROM staff WHERE user_id = ?', (user_id,))
        staff_id = cursor.fetchone()

        if not staff_id:
            return jsonify({"status": "error", "message": "User is not a staff member or not found."}), 404

        cursor.execute('''
            INSERT INTO check_in_out (staff_id, check_in_time)
            VALUES (?, datetime('now'))
        ''', (staff_id[0],))

        conn.commit()

        return jsonify({"status": "success", "message": "Check-in successful"}), 201

    except sqlite3.Error as e:
        conn.rollback()
        print(f"Database error: {str(e)}")  # Log the error to the console
        return jsonify({"status": "error", "message": "Database error"}), 500

    finally:
        conn.close()

@app.route('/check_session', methods=['POST'])
def check_session():
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('''
            SELECT u.user_id, u.username, u.role
            FROM sessions s
            JOIN users u ON s.user_id = u.user_id
            WHERE s.expires_at > datetime('now')
            ORDER BY s.created_at DESC
            LIMIT 1
        ''')

        session = cursor.fetchone()

        if session:
            user_data = {
                "user_id": session[0],
                "username": session[1],
                "role": session[2]
            }
            return jsonify({"status": "success", "session": user_data}), 200
        else:
            return jsonify({"status": "no_session", "message": "No active session found"}), 200

    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500

    finally:
        conn.close()

@app.route('/login', methods=['POST'])
def login():
    data = request.get_json()

    username = data.get('username')
    password_hash = data.get('password_hash')

    if not username or not password_hash:
        return jsonify({"status": "error", "message": "Username and password are required"}), 400

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('''
            SELECT user_id, password_hash, role FROM users WHERE username = ?
        ''', (username,))
        user = cursor.fetchone()

        if user:
            stored_hash = user[1]
            user_id = user[0]
            role = user[2]

            if password_hash == stored_hash:
                return jsonify({"status": "success", "user_id": user_id, "role": role}), 200
            else:
                return jsonify({"status": "error", "message": "Incorrect password"}), 401
        else:
            return jsonify({"status": "error", "message": "User not found"}), 404

    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500

    finally:
        conn.close()

@app.route('/log_audit', methods=['POST'])
def log_audit():
    data = request.get_json()

    username = data.get('username')
    action = data.get('action')

    if not username or not action:
        return jsonify({"status": "error", "message": "Username and action are required"}), 400

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('''
            INSERT INTO audit_logs (user_id, action, timestamp)
            VALUES ((SELECT user_id FROM users WHERE username = ?), ?, datetime('now'))
        ''', (username, action))

        conn.commit()

        return jsonify({"status": "success", "message": "Audit log entry created"}), 201

    except sqlite3.Error as e:
        conn.rollback()
        return jsonify({"status": "error", "message": str(e)}), 500

    finally:
        conn.close()

@app.route('/store_session', methods=['POST'])
def store_session():
    data = request.get_json()

    user_id = data.get('user_id')
    session_token = data.get('session_token')
    expires_at = data.get('expires_at')

    if not user_id or not session_token or not expires_at:
        return jsonify({"status": "error", "message": "User ID, session token, and expiration time are required"}), 400

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('''
            INSERT INTO sessions (user_id, session_token, expires_at)
            VALUES (?, ?, ?)
        ''', (user_id, session_token, expires_at))

        conn.commit()

        return jsonify({"status": "success", "message": "Session stored successfully"}), 201

    except sqlite3.Error as e:
        conn.rollback()
        return jsonify({"status": "error", "message": str(e)}), 500

    finally:
        conn.close()

@app.route('/staff_menu_items', methods=['GET'])
def get_menu_items():
    try:
        conn = get_db_connection()
        cur = conn.cursor()

        cur.execute("SELECT item_name, description, price, type, quantity FROM menuitems")
        menu_items = cur.fetchall()

        items = []
        for row in menu_items:
            items.append({
                'item_name': row['item_name'],
                'description': row['description'],
                'price': row['price'],
                'type': row['type'],
                'quantity': row['quantity']
            })

        return jsonify({"status": "success", "items": items}), 200

    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

    finally:
        conn.close()

@app.route('/order_menu_items', methods=['GET'])
def get_order_menu_items():
    item_type = request.args.get('type')

    if item_type not in ['food', 'drink']:
        return jsonify({"status": "error", "message": "Invalid type parameter"}), 400

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        # Include item_id in the SELECT statement
        cursor.execute('SELECT item_id, item_name, price, quantity FROM menuitems WHERE type = ?', (item_type,))
        menu_items = cursor.fetchall()

        items = []
        for row in menu_items:
            items.append({
                'item_id': row['item_id'],  # Added item_id
                'item_name': row['item_name'],
                'price': row['price'],
                'quantity': row['quantity']
            })

        return jsonify({"status": "success", "items": items}), 200

    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500

    finally:
        conn.close()


@app.route('/place_order_staff', methods=['POST'])
def place_order_staff():
    data = request.get_json()

    table_number = data.get("table_number")
    items = data.get("items", [])

    if not table_number:
        return jsonify({"status": "error", "message": "Table number is missing."}), 400
    if not items or not isinstance(items, list) or len(items) == 0:
        return jsonify({"status": "error", "message": "No items selected for the order."}), 400

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('INSERT INTO orders (table_id) VALUES (?)', (table_number,))
        order_id = cursor.lastrowid

        total_amount = 0.0

        for item in items:
            item_name = item.get("name")
            item_quantity = item.get("quantity")

            if not item_name or not isinstance(item_quantity, int) or item_quantity <= 0:
                return jsonify({"status": "error", "message": f"Invalid item or quantity for {item_name}."}), 400

            cursor.execute('SELECT item_id, price, quantity FROM menuitems WHERE item_name = ?', (item_name,))
            result = cursor.fetchone()

            if result:
                item_id, item_price, stock_quantity = result["item_id"], result["price"], result["quantity"]

                if stock_quantity >= item_quantity:
                    new_stock_quantity = stock_quantity - item_quantity
                    cursor.execute('UPDATE menuitems SET quantity = ? WHERE item_id = ?',
                                   (new_stock_quantity, item_id))

                    cursor.execute('INSERT INTO order_items (order_id, item_id, quantity) VALUES (?, ?, ?)',
                                   (order_id, item_id, item_quantity))

                    total_amount += item_price * item_quantity
                else:
                    return jsonify({"status": "error", "message": f"Not enough stock for {item_name}."}), 400
            else:
                return jsonify({"status": "error", "message": f"Item {item_name} not found."}), 400

        conn.commit()

        return jsonify({"status": "success", "total_amount": total_amount}), 200

    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500

    finally:
        conn.close()


@app.route('/add_menu_item', methods=['POST'])
def add_menu_item():
    data = request.get_json()

    item_name = data.get('item_name')
    description = data.get('description')
    price = data.get('price')
    type = data.get('type')
    quantity = data.get('quantity')

    if not all([item_name, description, price, type, quantity]):
        return jsonify({"status": "error", "message": "All fields are required."}), 400

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('''
            INSERT INTO menuitems (item_name, description, quantity, price, type)
            VALUES (?, ?, ?, ?, ?)
        ''', (item_name, description, quantity, price, type))

        conn.commit()
        return jsonify({"status": "success", "message": "Menu item added successfully."}), 201

    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500

    finally:
        if conn:
            conn.close()

@app.route('/remove_menu_item', methods=['POST'])
def remove_menu_item():
    data = request.get_json()

    item_name = data.get('item_name')
    type = data.get('type')

    if not all([item_name, type]):
        return jsonify({"status": "error", "message": "Item name and type are required."}), 400

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('''
            DELETE FROM menuitems
            WHERE item_name = ? AND type = ?
        ''', (item_name, type))

        conn.commit()

        if cursor.rowcount == 0:
            return jsonify({"status": "error", "message": "Item not found."}), 404

        return jsonify({"status": "success", "message": "Menu item removed successfully."}), 200

    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500

    finally:
        if conn:
            conn.close()

@app.route('/get_reserved_tables', methods=['GET'])
def get_reserved_tables():
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('''
            SELECT table_id
            FROM reservations
            WHERE status = 'reserved'
            AND DATE(reservation_time) = DATE('now')
        ''')

        reserved_tables = [row['table_id'] for row in cursor.fetchall()]

        return jsonify({
            "status": "success",
            "reserved_tables": reserved_tables
        }), 200

    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500

    finally:
        if conn:
            conn.close()

@app.route('/get_occupied_tables', methods=['GET'])
def get_occupied_tables():
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('SELECT table_number FROM tables WHERE is_occupied = 1')

        occupied_tables = cursor.fetchall()

        tables = [row['table_number'] for row in occupied_tables]

        return jsonify({"occupied_tables": tables}), 200

    except sqlite3.Error as e:
        return jsonify({"error": str(e)}), 500

    finally:
        if conn:
            conn.close()

@app.route('/get_reservations', methods=['GET'])
def get_reservations():
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('''
            SELECT reservation_id, name, table_id, guest_count, reservation_time
            FROM reservations
        ''')

        reservations = cursor.fetchall()

        reservation_list = [
            {
                "reservation_id": row["reservation_id"],
                "name": row["name"],
                "table_id": row["table_id"],
                "guest_count": row["guest_count"],
                "reservation_time": row["reservation_time"]
            } for row in reservations
        ]

        return jsonify(reservation_list), 200

    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500

    finally:
        conn.close()

@app.route('/remove_past_reservations', methods=['POST'])
def remove_past_reservations():
    data = request.get_json()

    if 'current_time' not in data:
        return jsonify({"status": "error", "message": "current_time is required"}), 400

    current_time = data['current_time']

    db = get_db_connection()
    if db is None:
        return jsonify({"status": "error", "message": "Database connection failed"}), 500

    try:
        cursor = db.cursor()
        cursor.execute("DELETE FROM reservations WHERE datetime(reservation_time) < datetime(?)", (current_time,))
        db.commit()

        return jsonify({"status": "success", "message": "Past reservations removed"}), 200
    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500
    finally:
        db.close()

@app.route('/get_available_tables', methods=['POST'])
def get_available_tables():
    data = request.get_json()

    if 'datetime' not in data:
        return jsonify({"status": "error", "message": "datetime is required"}), 400

    selected_datetime = data['datetime']

    db = get_db_connection()
    if db is None:
        return jsonify({"status": "error", "message": "Database connection failed"}), 500

    try:
        cursor = db.cursor()

        query = """
            SELECT table_number
            FROM tables
            WHERE table_number NOT IN (
                SELECT table_id
                FROM reservations
                WHERE DATE(reservation_time) = DATE(?)
            );
        """
        cursor.execute(query, (selected_datetime,))
        rows = cursor.fetchall()

        available_tables = [row['table_number'] for row in rows]

        return jsonify(available_tables), 200
    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500
    finally:
        db.close()

@app.route('/save_reservation', methods=['POST'])
def save_reservation():
    data = request.get_json()

    if not all(k in data for k in ("name", "table_id", "reservation_time", "guest_count")):
        return jsonify({"status": "error", "message": "Missing reservation details"}), 400

    name = data["name"]
    table_id = data["table_id"]
    reservation_time = data["reservation_time"]
    guest_count = data["guest_count"]

    # Get database connection
    db = get_db_connection()
    if db is None:
        return jsonify({"status": "error", "message": "Database connection failed"}), 500

    try:
        cursor = db.cursor()

        cursor.execute(
            """
            INSERT INTO reservations (name, table_id, reservation_time, guest_count, special_requests, status)
            VALUES (?, ?, ?, ?, '', 'reserved');
            """,
            (name, table_id, reservation_time, guest_count)
        )
        db.commit()

        return jsonify({"status": "success", "message": "Reservation saved successfully"}), 200
    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500
    finally:
        db.close()

@app.route('/get_occupied_tables_billing', methods=['GET'])
def get_occupied_tables_billing():
    db = get_db_connection()
    if db is None:
        return jsonify({"status": "error", "message": "Database connection failed"}), 500

    try:
        cursor = db.cursor()

        query = """
            SELECT DISTINCT tables.table_id, tables.table_number
            FROM tables
            JOIN orders ON tables.table_id = orders.table_id
            WHERE orders.is_paid = 0
        """
        cursor.execute(query)
        rows = cursor.fetchall()

        tables = [{"table_id": row["table_id"], "table_number": row["table_number"]} for row in rows]

        return jsonify(tables), 200
    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500
    finally:
        db.close()

@app.route('/get_latest_order/<int:table_id>', methods=['GET'])
def get_latest_order(table_id):
    db = get_db_connection()
    if db is None:
        return jsonify({"status": "error", "message": "Database connection failed"}), 500

    try:
        cursor = db.cursor()

        query = """
            SELECT menuitems.item_name, menuitems.price, order_items.quantity
            FROM order_items
            INNER JOIN menuitems ON menuitems.item_id = order_items.item_id
            INNER JOIN orders ON orders.order_id = order_items.order_id
            WHERE orders.order_id = (
                SELECT order_id
                FROM orders
                WHERE table_id = ? AND is_paid = 0
                ORDER BY order_time DESC
                LIMIT 1
            )
        """
        cursor.execute(query, (table_id,))
        rows = cursor.fetchall()

        order_items = [{"item_name": row["item_name"], "price": row["price"], "quantity": row["quantity"]} for row in rows]

        return jsonify(order_items), 200
    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500
    finally:
        db.close()

@app.route('/process_payment', methods=['POST'])
def process_payment():
    try:
        data = request.get_json()
        table_id = data.get('table_id')
        total_amount = data.get('total_amount')
        payment_method = data.get('payment_method')

        if not table_id or not total_amount or not payment_method:
            return jsonify({'status': 'error', 'message': 'Missing required payment data'}), 400

        conn = get_db_connection()
        cur = conn.cursor()

        cur.execute("""
            SELECT order_id FROM orders
            WHERE table_id = ? AND is_paid = 0
            ORDER BY order_time DESC LIMIT 1
        """, (table_id,))
        order = cur.fetchone()

        if order is None:
            return jsonify({'status': 'error', 'message': 'No unpaid order found for the given table'}), 404

        order_id = order['order_id']

        cur.execute("""
            INSERT INTO bills (order_id, total_amount, payment_method)
            VALUES (?, ?, ?)
        """, (order_id, total_amount, payment_method))

        cur.execute("""
            UPDATE orders SET is_paid = 1 WHERE order_id = ?
        """, (order_id,))

        conn.commit()
        conn.close()

        return jsonify({'status': 'success', 'message': 'Payment processed successfully'}), 200

    except Exception as e:
        return jsonify({'status': 'error', 'message': str(e)}), 500


@app.route('/load_staff', methods=['GET'])
def get_staff():
    conn = get_db_connection()
    staff = conn.execute('SELECT user_id, department, position, hire_date FROM staff ORDER BY user_id ASC').fetchall()
    conn.close()

    staff_list = []
    for row in staff:
        staff_list.append({
            "user_id": row["user_id"],
            "department": row["department"],
            "position": row["position"],
            "hire_date": row["hire_date"]
        })

    return jsonify(staff_list)

@app.route('/load_roles', methods=['GET'])
def load_roles():
    conn = get_db_connection()
    roles = conn.execute('SELECT role_name FROM roles WHERE role_id <> 5').fetchall()
    conn.close()

    roles_list = []
    for role in roles:
        roles_list.append({
            "role_name": role["role_name"]
        })

    return jsonify(roles_list)

@app.route('/get_next_user_id', methods=['GET'])
def get_next_user_id():
    conn = get_db_connection()
    cur = conn.cursor()
    cur.execute('SELECT MAX(user_id) FROM users')
    max_user_id = cur.fetchone()[0]
    conn.close()

    if max_user_id is None:
        next_user_id = 1
    else:
        next_user_id = max_user_id + 1

    return jsonify({"next_user_id": next_user_id})

@app.route('/add_staff', methods=['POST'])
def add_staff():
    with db_lock:
        try:
            data = request.get_json()
            user_id = data['user_id']
            department = data['department']
            role_name = data['position']
            hire_date = data['hire_date']

            conn = get_db_connection()
            cur = conn.cursor()

            cur.execute('INSERT INTO staff (user_id, department, position, hire_date) VALUES (?, ?, ?, ?)',
                        (user_id, department, role_name, hire_date))

            username = str(user_id)
            password_hash = hash_password(username)
            cur.execute('INSERT INTO users (user_id, username, role, password_hash, is_staff) VALUES (?, ?, ?, ?, ?)',
                        (user_id, username, role_name, password_hash, 1))

            cur.execute('SELECT role_id FROM roles WHERE role_name = ?', (role_name,))
            role_id = cur.fetchone()

            if role_id:
                cur.execute('INSERT INTO role_assignments (user_id, role_id) VALUES (?, ?)', (user_id, role_id[0]))

            conn.commit()

            return jsonify({"status": "success", "message": "Staff added successfully!"})

        except sqlite3.Error as e:
            conn.rollback()
            return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/get_permissions', methods=['POST'])
def get_permissions():
    role_id = ""
    raw_data = ""
    with db_lock:
        try:
            raw_data = request.get_data()
            print(f"Received raw request data: {raw_data}")

            data = request.get_json()
            if not data:
                return jsonify({"status": "error", "message": "Invalid or missing JSON payload"}), 400

            role_id = data.get('role_id')

            if not role_id:
                return jsonify({"status": "error", "message": "role_id is missing. Please provide a valid role_id."}), 400

            conn = get_db_connection()
            cur = conn.cursor()

            cur.execute('SELECT permission_name FROM permissions WHERE role_id = ?', (role_id,))
            permissions = cur.fetchall()

            conn.close()

            if not permissions:
                return jsonify({"status": "success", "permissions": []}), 200  # No permissions for the given role_id

            permissions_list = [permission[0] for permission in permissions]

            return jsonify({"status": "success", "permissions": permissions_list}), 200

        except Exception as e:
            return jsonify({"status": "error", "message": str(e) + str(role_id)}), 500

@app.route('/update_staff_member', methods=['POST'])
def update_staff_member():
    try:
        data = request.get_json()
        new_username = data['new_username']
        hashed_password = data['hashed_password']
        full_name = data['full_name']
        email = data['email']
        phone = data['phone']
        current_username = data['current_username']

        conn = get_db_connection()
        cur = conn.cursor()

        cur.execute('SELECT COUNT(*) FROM users WHERE username = ? AND username != ?', (new_username, current_username))
        if cur.fetchone()[0] > 0:
            return jsonify({"status": "error", "message": "Username already taken"}), 409

        cur.execute('''
            UPDATE users
            SET username = ?, password_hash = ?, full_name = ?, email = ?, phone = ?
            WHERE username = ?
        ''', (new_username, hashed_password, full_name, email, phone, current_username))

        conn.commit()
        conn.close()

        return jsonify({"status": "success", "message": "Staff member updated successfully"}), 200

    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/delete_reservation/<int:reservation_id>', methods=['DELETE'])
def delete_reservation(reservation_id):
    conn = get_db_connection()
    if conn is None:
        return jsonify({"status": "error", "message": "Database connection failed"}), 500

    try:
        cursor = conn.cursor()

        cursor.execute("DELETE FROM reservations WHERE reservation_id = ?", (reservation_id,))
        conn.commit()

        if cursor.rowcount == 0:
            return jsonify({"status": "error", "message": "Reservation not found"}), 404

        return jsonify({"status": "success", "message": "Reservation deleted successfully"}), 200

    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500

    finally:
        conn.close()

@app.route('/delete_staff', methods=['POST'])
def delete_staff():
    try:
        data = request.get_json()
        user_id = data.get('user_id')

        if not user_id:
            return jsonify({"status": "error", "message": "Missing user_id in request data"}), 400

        conn = get_db_connection()
        cur = conn.cursor()

        cur.execute('DELETE FROM staff WHERE user_id = ?', (user_id,))
        if cur.rowcount == 0:
            return jsonify({"status": "error", "message": "No staff member found with the given user_id"}), 404

        cur.execute('DELETE FROM users WHERE user_id = ?', (user_id,))
        if cur.rowcount == 0:
            return jsonify({"status": "error", "message": "No user found with the given user_id"}), 404

        cur.execute('DELETE FROM role_assignments WHERE user_id = ?', (user_id,))
        if cur.rowcount == 0:
            return jsonify({"status": "error", "message": "No role assignments found for the given user_id"}), 404

        conn.commit()
        conn.close()

        return jsonify({"status": "success", "message": "Staff member and associated data deleted successfully."})

    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/fetch_customer_orders', methods=['GET'])
def fetch_customer_orders():
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('''
            SELECT o.id AS order_id, o.customer_name, o.order_status, GROUP_CONCAT(m.item_name || ' (x' || coi.quantity || ')', ', ') AS order_details
            FROM customer_orders o
            LEFT JOIN customer_order_items coi ON o.id = coi.order_id
            LEFT JOIN menuitems m ON coi.item_id = m.item_id
            WHERE o.order_status NOT IN ('Completed', 'Declined')
            GROUP BY o.id
            ORDER BY o.created_at DESC
        ''')
        orders = cursor.fetchall()

        order_list = []
        for order in orders:
            order_data = {
                "order_id": order["order_id"],
                "customer_name": order["customer_name"],
                "order_details": order["order_details"],
                "order_status": order["order_status"]
            }
            order_list.append(order_data)

        conn.close()

        return jsonify(order_list), 200

    except sqlite3.Error as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/update_order_status', methods=['POST'])
def update_order_status():
    try:
        data = request.get_json()

        if not data:
            return jsonify({"status": "error", "message": "Missing JSON payload"}), 400

        order_id = data.get('order_id')
        new_status = data.get('status')

        if not order_id or not new_status:
            return jsonify({"status": "error", "message": "Missing order_id or status"}), 400

        conn = get_db_connection()
        cur = conn.cursor()

        cur.execute('''
            UPDATE customer_orders
            SET order_status = ?
            WHERE id = ?
        ''', (new_status, order_id))

        if cur.rowcount == 0:
            conn.close()
            return jsonify({"status": "error", "message": "Order ID not found"}), 404

        if new_status.lower() == "completed":
            cur.execute('SELECT item_id, quantity FROM customer_order_items WHERE order_id = ?', (order_id,))
            order_items = cur.fetchall()

            print(f"Order Items for Order ID {order_id}: {order_items}")

            for item in order_items:
                item_id = item['item_id']
                ordered_quantity = item['quantity']

                cur.execute('SELECT quantity FROM menuitems WHERE item_id = ?', (item_id,))
                stock_result = cur.fetchone()

                print(f"Stock for Item ID {item_id}: {stock_result}")

                if stock_result:
                    stock_quantity = stock_result['quantity']

                    stock_quantity = int(stock_quantity)
                    ordered_quantity = int(ordered_quantity)

                    print(f"Comparing Stock Quantity ({stock_quantity}) with Ordered Quantity ({ordered_quantity})")

                    if stock_quantity >= ordered_quantity:
                        new_stock_quantity = stock_quantity - ordered_quantity
                        cur.execute('UPDATE menuitems SET quantity = ? WHERE item_id = ?', (new_stock_quantity, item_id))

                        print(f"Updated Stock Quantity for Item ID {item_id}: {new_stock_quantity}")

                    else:
                        conn.rollback()
                        conn.close()
                        return jsonify({"status": "error", "message": f"Insufficient stock for item ID {item_id}."}), 400
                else:
                    conn.rollback()
                    conn.close()
                    return jsonify({"status": "error", "message": f"Item ID {item_id} not found."}), 404

        conn.commit()
        conn.close()

        return jsonify({"status": "success", "message": "Order status updated"}), 200

    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/update_profile', methods=['POST'])
def update_profile():
    try:
        data = request.get_json()

        print("Received data:", data)

        if not data or 'user_id' not in data:
            return jsonify({"error": "User ID is required"}), 400

        user_id = data.get('user_id')

        fields_to_update = ['full_name', 'email', 'phone']
        updates = {}

        for field in fields_to_update:
            if field in data and data[field]:
                updates[field] = data[field]

        if not updates:
            return jsonify({"error": "No valid fields provided to update"}), 400

        update_query = "UPDATE users SET "
        update_query += ", ".join([f"{field} = ?" for field in updates])
        update_query += " WHERE user_id = ?"

        conn = get_db_connection()
        conn.execute(update_query, list(updates.values()) + [user_id])
        conn.commit()
        conn.close()

        return jsonify({"message": "Profile updated successfully"}), 200

    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/create_table_overview', methods=['GET'])
def create_table_overview():
    try:
        conn = get_db_connection()
        cursor = conn.cursor()
        cursor.execute("SELECT table_number FROM tables")
        tables = cursor.fetchall()
        conn.close()

        table_list = [{"table_number": row[0]} for row in tables]
        return jsonify(table_list)

    except sqlite3.Error as e:
        return jsonify({"error": str(e)}), 500


@app.route('/update_table_occupied_status', methods=['POST'])
def update_table_occupied_status():
    try:
        data = request.get_json()
        table_number = data['table_number']
        is_occupied = data['is_occupied']

        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute("UPDATE tables SET is_occupied = ? WHERE table_number = ?", (is_occupied, table_number))
        conn.commit()

        if cursor.rowcount == 0:
            return jsonify({"error": "Table not found"}), 404

        conn.close()

        return jsonify({"message": "Table status updated successfully"}), 200

    except sqlite3.Error as e:
        return jsonify({"error": str(e)}), 500
    except KeyError:
        return jsonify({"error": "Invalid input"}), 400

@app.route('/check_reserved_tables', methods=['GET'])
def check_reserved_tables():
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute("SELECT table_id FROM reservations WHERE status = 'reserved' AND DATE(reservation_time) = DATE('now')")
        reserved_tables = cursor.fetchall()
        conn.close()

        reserved_table_list = [row[0] for row in reserved_tables]
        return jsonify(reserved_table_list)

    except sqlite3.Error as e:
        return jsonify({"error": str(e)}), 500

@app.route('/check_occupied_tables', methods=['GET'])
def check_occupied_tables():
    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute("SELECT table_id FROM tables WHERE is_occupied = 1")
        occupied_tables = cursor.fetchall()
        conn.close()

        occupied_table_list = [row[0] for row in occupied_tables]
        return jsonify(occupied_table_list)

    except sqlite3.Error as e:
        return jsonify({"error": str(e)}), 500

# @app.route('/generate_qr_code', methods=['POST'])
# def generate_qr_code():
#     try:
#         data = request.get_json()
#         order_id = data.get('order_id')

#         if not order_id:
#             return jsonify({"status": "error", "message": "Missing order_id"}), 400

#         # Generate the QR code containing the verification URL
#         verification_url = f"http://Damjan03.pythonanywhere.com/verify_qr_payment/{order_id}"
#         qr = qrcode.QRCode(
#             version=1,
#             error_correction=qrcode.constants.ERROR_CORRECT_L,
#             box_size=10,
#             border=4,
#         )
#         qr.add_data(verification_url)
#         qr.make(fit=True)

#         img = qr.make_image(fill_color="black", back_color="white")

#         # Save the image to a byte stream
#         byte_io = io.BytesIO()
#         img.save(byte_io, 'PNG')
#         byte_io.seek(0)

#         return send_file(byte_io, mimetype='image/png')

#     except Exception as e:
#         return jsonify({"status": "error", "message": str(e)}), 500


# @app.route('/verify_qr_payment/<int:order_id>', methods=['GET'])
# def verify_qr_payment(order_id):
#     try:
#         # Connect to the database
#         conn = get_db_connection()
#         cursor = conn.cursor()

#         # Debugging: Print the incoming order_id
#         print(f"Verifying QR payment for order ID: {order_id}")

#         # Check if the order exists and is unpaid
#         cursor.execute('''
#             SELECT * FROM orders WHERE order_id = ? AND is_paid = 0
#         ''', (order_id,))

#         order = cursor.fetchone()

#         if not order:
#             # If the order is not found or already paid
#             print(f"Order ID {order_id} not found or already paid.")
#             conn.close()
#             return jsonify({"status": "error", "message": "Order not found or already paid"}), 404

#         # If the order exists and is unpaid, mark it as paid
#         cursor.execute('''
#             UPDATE orders
#             SET is_paid = 1
#             WHERE order_id = ?
#         ''', (order_id,))

#         # Add the payment to the bills table
#         cursor.execute('''
#             INSERT INTO bills (order_id, total_amount, payment_method)
#             VALUES (?, ?, ?)
#         ''', (order_id, order['total_amount'], "QR Code"))

#         # Commit changes and close connection
#         conn.commit()
#         conn.close()

#         # Return success response after successful payment
#         print(f"Payment verified successfully for order ID: {order_id}")
#         return jsonify({"status": "success", "message": "Payment verified and processed successfully"}), 200

#     except sqlite3.Error as e:
#         # Handle database errors
#         print(f"Database error: {str(e)}")
#         return jsonify({"status": "error", "message": str(e)}), 500


# @app.route('/get_current_order_id/<int:table_id>', methods=['GET'])
# def get_current_order_id(table_id):
#     db = get_db_connection()
#     if db is None:
#         return jsonify({"status": "error", "message": "Database connection failed"}), 500

#     try:
#         cursor = db.cursor()

#         # Query to get the latest unpaid order ID for the given table
#         query = """
#             SELECT order_id
#             FROM orders
#             WHERE table_id = ? AND is_paid = 0
#             ORDER BY order_time DESC
#             LIMIT 1
#         """
#         cursor.execute(query, (table_id,))
#         row = cursor.fetchone()

#         # Check if an order was found
#         if row is None:
#             return jsonify({"status": "error", "message": "No unpaid orders found for this table"}), 404

#         # Return the order_id
#         order_id = row["order_id"]
#         return jsonify({"order_id": order_id}), 200

#     except sqlite3.Error as e:
#         return jsonify({"status": "error", "message": str(e)}), 500

#     finally:
#         db.close()

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000)
