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

import com.google.common.collect.AbstractIterator;

import java.util.Iterator;

import static com.google.common.base.Preconditions.checkArgument;
import static tpch.prestosql.GenerateUtils.calculateRowCount;
import static tpch.prestosql.GenerateUtils.calculateStartIndex;
import static java.util.Locale.ENGLISH;
import static java.util.Objects.requireNonNull;

public class CustomerGenerator implements Iterable<Customer>
{
    public static final int SCALE_BASE = 150_000;

    private static final int ACCOUNT_BALANCE_MIN = -99999;
    private static final int ACCOUNT_BALANCE_MAX = 999999;

    /** 地址平均字符长度 */
    private static final int ADDRESS_AVERAGE_LENGTH = 25;
    /** 备注平均字符长度 */
    private static final int COMMENT_AVERAGE_LENGTH = 73;

    /**
     * 比例因子.
     *
     * 根据比例因子Scale Factor（SF）的大小确定数据集的大小，
     * 当SF=1时，表的大小为1GB；
     * SF可取值有（1,10,30,100,300,1000,3000,10000）。
     */
    private final double scaleFactor;
    private final int part;
    private final int partCount;

    private final Distributions distributions;
    private final TextPool textPool;

    /**
     *
     * @param scaleFactor 比例因子
     * @param part 第几部份的客户
     * @param partCount 将客户划分成SCALE_BASE*scaleFactor/partCount份
     */
    public CustomerGenerator(double scaleFactor, int part, int partCount)
    {
        this(scaleFactor, part, partCount, Distributions.getDefaultDistributions(), TextPool.getDefaultTestPool());
    }

    public CustomerGenerator(double scaleFactor, int part, int partCount, Distributions distributions, TextPool textPool)
    {
        checkArgument(scaleFactor > 0, "scaleFactor must be greater than 0");
        checkArgument(part >= 1, "part must be at least 1");
        checkArgument(part <= partCount, "part must be less than or equal to part count");

        this.scaleFactor = scaleFactor;
        this.part = part;
        this.partCount = partCount;

        this.distributions = requireNonNull(distributions, "distributions is null");
        this.textPool = requireNonNull(textPool, "textPool is null");
    }

    @Override
    public Iterator<Customer> iterator()
    {
        return new CustomerGeneratorIterator(
            distributions,
            textPool,
            calculateStartIndex(SCALE_BASE, scaleFactor, part, partCount),
            calculateRowCount(SCALE_BASE, scaleFactor, part, partCount)
        );
    }

    private static class CustomerGeneratorIterator extends AbstractIterator<Customer>
    {
        private final RandomAlphaNumeric addressRandom = new RandomAlphaNumeric(881155353, ADDRESS_AVERAGE_LENGTH);
        private final RandomBoundedInt nationKeyRandom;
        private final RandomPhoneNumber phoneRandom = new RandomPhoneNumber(1521138112);
        private final RandomBoundedInt accountBalanceRandom = new RandomBoundedInt(298370230, ACCOUNT_BALANCE_MIN, ACCOUNT_BALANCE_MAX);
        private final RandomString marketSegmentRandom;
        private final RandomText commentRandom;

        /** 起始客户偏移量 */
        private final long startIndex;
        /** 多少个客户 */
        private final long rowCount;
        /** 当前迭代到第几个客户 */
        private long index;

        private CustomerGeneratorIterator(Distributions distributions, TextPool textPool, long startIndex, long rowCount)
        {
            this.startIndex = startIndex;
            this.rowCount = rowCount;

            nationKeyRandom = new RandomBoundedInt(1489529863, 0, distributions.getNations().size() - 1);
            marketSegmentRandom = new RandomString(1140279430, distributions.getMarketSegments());
            commentRandom = new RandomText(1335826707, textPool, COMMENT_AVERAGE_LENGTH);

            addressRandom.advanceRows(startIndex);
            nationKeyRandom.advanceRows(startIndex);
            phoneRandom.advanceRows(startIndex);
            accountBalanceRandom.advanceRows(startIndex);
            marketSegmentRandom.advanceRows(startIndex);
            commentRandom.advanceRows(startIndex);
        }

        @Override
        protected Customer computeNext()
        {
            if (index >= rowCount) {
                return endOfData();
            }

            Customer customer = makeCustomer(startIndex + index + 1);

            addressRandom.rowFinished();
            nationKeyRandom.rowFinished();
            phoneRandom.rowFinished();
            accountBalanceRandom.rowFinished();
            marketSegmentRandom.rowFinished();
            commentRandom.rowFinished();

            index++;

            return customer;
        }

        private Customer makeCustomer(long customerKey)
        {
            long nationKey = nationKeyRandom.nextValue();

            return new Customer(
                customerKey,
                customerKey,
                String.format(ENGLISH, "Customer#%09d", customerKey),
                addressRandom.nextValue(),
                nationKey,
                phoneRandom.nextValue(nationKey),
                accountBalanceRandom.nextValue(),
                marketSegmentRandom.nextValue(),
                commentRandom.nextValue()
            );
        }
    }
}
