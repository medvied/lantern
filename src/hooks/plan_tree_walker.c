#include <postgres.h>

#include "plan_tree_walker.h"

#include <miscadmin.h>
#include <nodes/nodeFuncs.h>
#include <nodes/nodes.h>
#include <nodes/parsenodes.h>
#include <nodes/plannodes.h>

bool base_plan_walker(Plan *plan, bool (*walker_func)(Node *plan, void *context), void *context)
{
    if(walker_func((Node *)plan->targetlist, context)) return true;
    if(walker_func((Node *)plan->qual, context)) return true;
    if(walker_func((Node *)plan->lefttree, context)) return true;
    if(walker_func((Node *)plan->righttree, context)) return true;
    if(walker_func((Node *)plan->initPlan, context)) return true;
    return false;
}

bool plan_tree_walker(Plan *plan, bool (*walker_func)(Node *plan, void *context), void *context)
{
    check_stack_depth();

    switch(nodeTag(plan)) {
        case T_SeqScan:
        {
            SeqScan *seqscan = (SeqScan *)plan;
#if PG_VERSION_NUM >= 150000
            Plan seqscanplan = seqscan->scan.plan;
#else
            Plan seqscanplan = seqscan->plan;
#endif
            if(base_plan_walker(&seqscanplan, walker_func, context)) return true;
            break;
        }
        case T_IndexScan:
        {
            IndexScan *indexscan = (IndexScan *)plan;
            if(base_plan_walker(&(indexscan->scan.plan), walker_func, context)) return true;
            if(walker_func((Node *)indexscan->indexqual, context)) return true;
            if(walker_func((Node *)indexscan->indexorderby, context)) return true;
            break;
        }
        case T_IndexOnlyScan:
        {
            IndexOnlyScan *indexonlyscan = (IndexOnlyScan *)plan;
            if(base_plan_walker(&(indexonlyscan->scan.plan), walker_func, context)) return true;
            if(walker_func((Node *)indexonlyscan->indexqual, context)) return true;
            if(walker_func((Node *)indexonlyscan->indexorderby, context)) return true;
            break;
        }
        case T_SubqueryScan:
        {
            SubqueryScan *subqueryscan = (SubqueryScan *)plan;
            if(base_plan_walker(&(subqueryscan->scan.plan), walker_func, context)) return true;
            if(walker_func((Node *)subqueryscan->subplan, context)) return true;
            break;
        }
        case T_CteScan:
        {
            CteScan *ctescan = (CteScan *)plan;
            if(base_plan_walker(&(ctescan->scan.plan), walker_func, context)) return true;
            break;
        }
#if PG_VERSION_NUM < 160000
        case T_Join:
        {
            Join *join = (Join *)plan;
            if(base_plan_walker(&(join->plan), walker_func, context)) return true;
            if(walker_func((Node *)join->joinqual, context)) return true;
            break;
        }
#endif
        case T_Agg:
        {
            Agg *agg = (Agg *)plan;
            if(base_plan_walker(&(agg->plan), walker_func, context)) return true;
            break;
        }
        case T_Group:
        {
            Group *group = (Group *)plan;
            if(base_plan_walker(&(group->plan), walker_func, context)) return true;
            break;
        }
        case T_Sort:
        {
            Sort *sort = (Sort *)plan;
            if(base_plan_walker(&(sort->plan), walker_func, context)) return true;
            break;
        }
        case T_Unique:
        {
            Unique *unique = (Unique *)plan;
            if(base_plan_walker(&(unique->plan), walker_func, context)) return true;
            break;
        }
        case T_NestLoop:
        {
            NestLoop *nestloop = (NestLoop *)plan;
            if(base_plan_walker((Plan *)&(nestloop->join), walker_func, context)) return true;
            break;
        }
        case T_Result:
        {
            Result *result = (Result *)plan;
            if(base_plan_walker(&(result->plan), walker_func, context)) return true;
            if(walker_func((Node *)result->resconstantqual, context)) return true;
            break;
        }
        case T_Limit:
        {
            Limit *limit = (Limit *)plan;
            if(base_plan_walker(&(limit->plan), walker_func, context)) return true;
            if(walker_func((Node *)limit->limitOffset, context)) return true;
            if(walker_func((Node *)limit->limitCount, context)) return true;
            break;
        }
        case T_Append:
        {
            Append *append = (Append *)plan;
            if(base_plan_walker(&(append->plan), walker_func, context)) return true;
            if(walker_func((Node *)append->appendplans, context)) return true;
            break;
        }
        default:
            return false;
    }
    return false;
}
