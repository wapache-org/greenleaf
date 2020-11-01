/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package tpch.prestosql;

import static tpch.prestosql.TpchColumnTypes.DOUBLE;
import static tpch.prestosql.TpchColumnTypes.IDENTIFIER;
import static tpch.prestosql.TpchColumnTypes.varchar;

public enum CustomerColumn implements TpchColumn<Customer>
{
    CUSTOMER_KEY("c_custkey", IDENTIFIER) {
        @Override
        public long getIdentifier(Customer customer)
        {
            return customer.getCustomerKey();
        }
    },

    NAME("c_name", varchar(25)) {
        @Override
        public String getString(Customer customer)
        {
            return customer.getName();
        }
    },

    ADDRESS("c_address", varchar(40)) {
        @Override
        public String getString(Customer customer)
        {
            return customer.getAddress();
        }
    },

    NATION_KEY("c_nationkey", IDENTIFIER) {
        @Override
        public long getIdentifier(Customer customer)
        {
            return customer.getNationKey();
        }
    },

    PHONE("c_phone", varchar(15)) {
        @Override
        public String getString(Customer customer)
        {
            return customer.getPhone();
        }
    },

    ACCOUNT_BALANCE("c_acctbal", DOUBLE) {
        @Override
        public double getDouble(Customer customer)
        {
            return customer.getAccountBalance();
        }

        @Override
        public long getIdentifier(Customer customer)
        {
            return customer.getAccountBalanceInCents();
        }
    },

    MARKET_SEGMENT("c_mktsegment", varchar(10)) {
        @Override
        public String getString(Customer customer)
        {
            return customer.getMarketSegment();
        }
    },

    COMMENT("c_comment", varchar(117)) {
        @Override
        public String getString(Customer customer)
        {
            return customer.getComment();
        }
    };

    private final String columnName;
    private final TpchColumnType type;

    CustomerColumn(String columnName, TpchColumnType type)
    {
        this.columnName = columnName;
        this.type = type;
    }

    @Override
    public String getColumnName()
    {
        return columnName;
    }

    @Override
    public TpchColumnType getType()
    {
        return type;
    }

    @Override
    public double getDouble(Customer customer)
    {
        throw new UnsupportedOperationException();
    }

    @Override
    public long getIdentifier(Customer customer)
    {
        throw new UnsupportedOperationException();
    }

    @Override
    public int getInteger(Customer customer)
    {
        throw new UnsupportedOperationException();
    }

    @Override
    public String getString(Customer customer)
    {
        throw new UnsupportedOperationException();
    }

    @Override
    public int getDate(Customer entity)
    {
        throw new UnsupportedOperationException();
    }
}
