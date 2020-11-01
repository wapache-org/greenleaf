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

import static tpch.prestosql.GenerateUtils.formatMoney;
import static java.util.Locale.ENGLISH;
import static java.util.Objects.requireNonNull;

/**
 * <code>
 * -- 消费者
 * CREATE TABLE customer
 * (
 *     c_custkey     BIGINT not null,
 *     c_name        VARCHAR(25) not null,
 *     c_address     VARCHAR(40) not null,
 *     c_nationkey   INTEGER not null,
 *     c_phone       CHAR(15) not null,
 *     c_acctbal     DOUBLE PRECISION   not null,
 *     c_mktsegment  CHAR(10) not null,
 *     c_comment     VARCHAR(117) not null
 * );
 * </code>
 */
public class Customer implements TpchEntity
{
    private final long rowNumber;

    private final long customerKey;
    private final String name;
    private final String address;
    private final long nationKey;
    private final String phone;
    private final long accountBalance;
    private final String marketSegment;
    private final String comment;

    public Customer(long rowNumber, long customerKey, String name, String address, long nationKey, String phone, long accountBalance, String marketSegment, String comment)
    {
        this.rowNumber = rowNumber;
        this.customerKey = customerKey;
        this.name = requireNonNull(name, "name is null");
        this.address = requireNonNull(address, "address is null");
        this.nationKey = nationKey;
        this.phone = requireNonNull(phone, "phone is null");
        this.accountBalance = accountBalance;
        this.marketSegment = requireNonNull(marketSegment, "marketSegment is null");
        this.comment = requireNonNull(comment, "comment is null");
    }

    @Override
    public long getRowNumber()
    {
        return rowNumber;
    }

    public long getCustomerKey()
    {
        return customerKey;
    }

    public String getName()
    {
        return name;
    }

    public String getAddress()
    {
        return address;
    }

    public long getNationKey()
    {
        return nationKey;
    }

    public String getPhone()
    {
        return phone;
    }

    public double getAccountBalance()
    {
        return accountBalance / 100.0;
    }

    public long getAccountBalanceInCents()
    {
        return accountBalance;
    }

    public String getMarketSegment()
    {
        return marketSegment;
    }

    public String getComment()
    {
        return comment;
    }

    @Override
    public String toLine()
    {
        return String.format(ENGLISH,
                "%d|%s|%s|%d|%s|%s|%s|%s",
                customerKey,
                name,
                address,
                nationKey,
                phone,
                formatMoney(accountBalance),
                marketSegment,
                comment);
    }
}
