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

import org.junit.Test;
import tpch.prestosql.NationGenerator;

import static tpch.GeneratorAssertions.assertEntityLinesMD5;

public class TestNationGenerator
{
    @SuppressWarnings("SpellCheckingInspection")
    @Test
    public void testGenerator()
    {
        assertEntityLinesMD5(new NationGenerator(), "2f588e0b7fa72939b498c2abecd9fbbe");
    }
}
